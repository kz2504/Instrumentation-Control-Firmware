#include "main.h"
#include "schedule.h"
#include "interface.h"
#include "usbd_cdc_if.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

volatile uint16_t head = 0;
volatile uint16_t tail = 0;

//Temporary triggering
//Start
static volatile uint8_t  g_pa0_pulse_active = 0;
static volatile uint32_t g_pa0_pulse_start_ms = 0;

static void pa0_pulse_start_1s(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    g_pa0_pulse_active = 1;
    g_pa0_pulse_start_ms = HAL_GetTick();
}

static void pa0_pulse_tick(void)
{
    if (!g_pa0_pulse_active) return;

    uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - g_pa0_pulse_start_ms) >= 1000u) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
        g_pa0_pulse_active = 0;
    }
}
//End

uint8_t buffer[BUFFER_SIZE];

uint16_t get_next(uint16_t i) {
    return (uint16_t)((i + 1u) % BUFFER_SIZE);
}

int buffer_push(uint8_t byte) { //1 if push success, 0 if buffer full
    uint16_t next = get_next(head);
    if (next == tail) {
    	return 0;
    }
    buffer[head] = byte;
    head = next;
    return 1;
}

int buffer_pop(uint8_t *out)
{
    if (tail == head) {
        return 0;
    }
    *out = buffer[tail];
    tail = get_next(tail);
    return 1;
}

static void usb_send_line(const char *s)
{
    const uint32_t max_tries = 2000;
    uint32_t tries = 0;

    uint16_t len = (uint16_t)strlen(s);
    while (tries++ < max_tries) {
        if (CDC_Transmit_FS((uint8_t*)s, len) == USBD_OK) return;
        for (volatile int i = 0; i < 200; i++) { __NOP(); }
    }
}

static void usb_sendf(const char *fmt, ...)
{
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    size_t n = strlen(buf);
    if (n == 0 || buf[n-1] != '\n') {
        if (n < sizeof(buf)-1) {
            buf[n] = '\n';
            buf[n+1] = '\0';
        }
    }
    usb_send_line(buf);
}

int split_tokens(char *s, char *argv[], int max_tokens) {
    int argc = 0;
    while (*s && argc < max_tokens) {
        while (*s && isspace((unsigned char)*s)) s++;
        if (!*s) break;
        argv[argc++] = s;
        while (*s && !isspace((unsigned char)*s)) s++;
        if (*s) *s++ = '\0';
    }
    return argc;
}


int parse_i32(const char *s, int32_t *out) {
    char *end;
    long v = strtol(s, &end, 10);
    if (end == s) return 0;
    *out = (int32_t)v;
    return 1;
}

int parse_f32(const char *s, float *out) {
    char *end;
    float v = strtof(s, &end);
    if (end == s) return 0;
    *out = v;
    return 1;
}

typedef struct __attribute__((packed)) {
    uint8_t  block_index;
    uint32_t duration_ms;

    uint8_t state_mask; //bit i = pump i on/off
    uint8_t dir_mask; //bit i = pump i dir

    uint16_t flow[NUM_PUMPS]; //uL/min

    uint8_t stim_on;
    uint16_t stim_freq_x100; //Hz * 100
    uint16_t stim_duty_x10; //% * 10  (50.0 -> 500)
} block_msg_t;


void handle_line(char *line, uint8_t card) {
    char *argv[MAX_TOKENS];
    int argc = split_tokens(line, argv, MAX_TOKENS);
    if (argc == 0) return;

    if (strcmp(argv[0], "CLR_SCH") == 0) {
        int ok = spi_send_clear(card);
        usb_sendf(ok ? "OK CLR_SCH" : "ERR CLR_SCH SPI");
        return;
    }

    if (strcmp(argv[0], "SCH_STRT") == 0) {
        int ok = spi_send_start(card);
        usb_sendf(ok ? "OK SCH_STRT" : "ERR SCH_STRT SPI");
        return;
    }

    if (strcmp(argv[0], "SCH_STOP") == 0) {
        int ok = spi_send_stop(card);
        usb_sendf(ok ? "OK SCH_STOP" : "ERR SCH_STOP SPI");
        return;
    }

    if (strcmp(argv[0], "TRIG_RDY") == 0) {
        if (argc < 2) { usb_sendf("ERR TRIG_RDY ARGS"); return; }

        int32_t r;
        if (!parse_i32(argv[1], &r)) { usb_sendf("ERR TRIG_RDY PARSE"); return; }

        int ok = spi_send_trigger_ready(card, (uint8_t)(r ? 1 : 0));
        usb_sendf(ok ? "OK TRIG_RDY %ld" : "ERR TRIG_RDY SPI", (long)r);
        return;
    }

    if (strcmp(argv[0], "BLOCK") == 0) {
        const int need = 3 + 3 * NUM_PUMPS + 3;
        if (argc < need) { usb_sendf("ERR BLOCK ARGS"); return; }

        int32_t idx_i, s_i, d_i, stim_on_i;
        float duration_s, flow_f, stim_freq_f, stim_duty_f;

        if (!parse_i32(argv[1], &idx_i)) { usb_sendf("ERR BLOCK IDX"); return; }
        if (!parse_f32(argv[2], &duration_s)) { usb_sendf("ERR BLOCK DUR"); return; }

        block_msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.block_index = (uint8_t)idx_i;

        if (duration_s < 0) duration_s = 0;
        msg.duration_ms = (uint32_t)(duration_s * 1000.0f + 0.5f);

        int k = 3;
        for (int i = 0; i < NUM_PUMPS; i++) {
            if (!parse_i32(argv[k++], &s_i)) { usb_sendf("ERR BLOCK PSTATE"); return; }
            if (!parse_f32(argv[k++], &flow_f)) { usb_sendf("ERR BLOCK PFLOW"); return; }
            if (!parse_i32(argv[k++], &d_i)) { usb_sendf("ERR BLOCK PDIR"); return; }

            if (s_i) msg.state_mask |= (1u << i);
            if (d_i) msg.dir_mask |= (1u << i);

            if (flow_f < 0) flow_f = 0;
            if (flow_f > 65535.0f) flow_f = 65535.0f;
            msg.flow[i] = (uint16_t)(flow_f + 0.5f);
        }

        if (!parse_i32(argv[k++], &stim_on_i)) { usb_sendf("ERR BLOCK STIMON"); return; }
        if (!parse_f32(argv[k++], &stim_freq_f)) { usb_sendf("ERR BLOCK STIMF"); return; }
        if (!parse_f32(argv[k++], &stim_duty_f)) { usb_sendf("ERR BLOCK STIMD"); return; }

        msg.stim_on = (uint8_t)(stim_on_i ? 1 : 0);

        if (stim_freq_f < 0) stim_freq_f = 0;
        float sf = stim_freq_f * 100.0f;
        if (sf > 65535.0f) sf = 65535.0f;
        msg.stim_freq_x100 = (uint16_t)(sf + 0.5f);

        if (stim_duty_f < 0) stim_duty_f = 0;
        if (stim_duty_f > 100.0f) stim_duty_f = 100.0f;
        float sd = stim_duty_f * 10.0f;
        if (sd > 65535.0f) sd = 65535.0f;
        msg.stim_duty_x10 = (uint16_t)(sd + 0.5f);

        pa0_pulse_start_1s(); //Trigger
        int ok = spi_send_block(card, &msg, sizeof(msg));
        usb_sendf(ok ? "OK BLOCK %ld" : "ERR BLOCK SPI %ld", (long)idx_i);
        return;
    }

    usb_sendf("ERR UNKNOWN %s", argv[0]);
}

void usb_protocol_poll(uint8_t card) {
	pa0_pulse_tick(); //Trigger

    static char line[LINE_MAX];
    static uint16_t n = 0;
    static uint8_t discarding = 0;

    uint8_t b;
    while (buffer_pop(&b)) {
        if (b == '\r') continue;

        if (b == '\n') {
            if (!discarding) {
                line[n] = '\0';
                if (n > 0) handle_line(line, card);
            }
            n = 0;
            discarding = 0;
            continue;
        }

        if (discarding) continue;

        if (n < (LINE_MAX - 1)) {
            line[n++] = (char)b;
        } else {
            n = 0;
            discarding = 1;
        }
    }
}
