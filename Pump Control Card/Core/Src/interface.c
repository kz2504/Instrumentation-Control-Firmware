#include "interface.h"
#include "main.h"
#include "spi.h"
#include "schedule.h"
#include <string.h>
#include <stdint.h>

#define CMD_CLEAR 0x01
#define CMD_START 0x02
#define CMD_STOP  0x03
#define CMD_BLOCK  0x10
#define CMD_TRIGGER_RDY 0x04

volatile uint32_t g_spi_frame_count = 0;
volatile uint32_t g_spi_error_count = 0;
volatile uint8_t g_spi_last_cmd = 0;
volatile uint8_t g_spi_last_len = 0;
volatile uint8_t g_spi_last_payload[PUMP_MAX_PAYLOAD];

static uint8_t payload[PUMP_MAX_PAYLOAD];

#define RX_TIMEOUT 2

static void handle_frame(uint8_t cmd, const uint8_t *p, uint8_t len)
{
    switch (cmd) {
        case CMD_CLEAR:
            schedule_clear();
            break;

        case CMD_START:
            schedule_start(HAL_GetTick());
            break;

        case CMD_STOP:
            schedule_stop();
            break;

        case CMD_BLOCK:
            if (len == sizeof(block_msg_t)) {
                block_msg_t b;
                memcpy(&b, p, sizeof(b));
                (void)schedule_store_block(&b);
            }
            break;

        case CMD_TRIGGER_RDY:
            //External trigger logic
            break;

        default:
            break;
    }
}

void pump_spi_nack_init(void)
{
    g_spi_frame_count = 0;
    g_spi_error_count = 0;
    g_spi_last_cmd = 0;
    g_spi_last_len = 0;
    memset((void*)g_spi_last_payload, 0, sizeof(g_spi_last_payload));
}

void pump_spi_nack_poll(void)
{
    uint8_t hdr[2] = {0,0};

    if (HAL_SPI_Receive(&hspi1, hdr, 2, RX_TIMEOUT) != HAL_OK) {
        g_spi_error_count++;
        return;
    }

    uint8_t cmd = hdr[0];
    uint8_t len = hdr[1];

    if (len > PUMP_MAX_PAYLOAD) {
        g_spi_error_count++;
        return;
    }

    if (len > 0) {
        if (HAL_SPI_Receive(&hspi1, payload, len, RX_TIMEOUT) != HAL_OK) {
            g_spi_error_count++;
            return;
        }
        memcpy((void*)g_spi_last_payload, payload, len);
    }

    g_spi_last_cmd = cmd;
    g_spi_last_len = len;
    g_spi_frame_count++;

    handle_frame(cmd, payload, len);
}
