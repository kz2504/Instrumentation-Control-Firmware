#include "schedule.h"
#include <string.h>
#include "shiftreg.h"
#include "DAC.h"

#define AVDD_VOLTS 4.87f
#define V_MIN_VOLTS 0.00f
#define V_MAX_VOLTS 4.70f

#define FLOW_AT_VMIN_ULPM 0.0f
#define FLOW_AT_VMAX_ULPM 1000.0f
#define DAC_MODE_DEFAULT 0

static void pump_set_flow(uint8_t i, uint16_t flow);
static void hw_stim_set(uint8_t on, uint16_t freq_x100, uint16_t duty_x10);

static block_msg_t g_blocks[MAX_BLOCKS];
static uint8_t g_block_valid[MAX_BLOCKS];

//Debug state
volatile uint8_t g_sched_running = 0;
volatile uint8_t g_sched_cur_index = 0;
volatile uint32_t g_sched_block_start_ms = 0;
volatile uint32_t g_sched_blocks_loaded = 0;

static void apply_block(const block_msg_t *b) {
	shiftByteEN(0x00);
	shiftByteDIR(b->dir_mask);
	shiftByteDIR(b->dir_mask);
    for (uint8_t i = 0; i < NUM_PUMPS; i++) {
        pump_set_flow(i, b->flow[i]);
    }
    shiftByteEN(b->state_mask);
    shiftByteEN(b->state_mask);
    hw_stim_set(b->stim_on, b->stim_freq_x100, b->stim_duty_x10);
}

void schedule_clear(void)
{
    memset(g_block_valid, 0, sizeof(g_block_valid));
    memset(g_blocks, 0, sizeof(g_blocks));
    g_sched_blocks_loaded = 0;

    schedule_stop();
}

static void all_off(void)
{
    shiftByteEN(0x00);
    for (uint8_t i = 0; i < NUM_PUMPS; i++) {
        writeDAC(i, AVDD_VOLTS, V_MIN_VOLTS, DAC_MODE_DEFAULT);
    }
    hw_stim_set(0, 0, 0);
}


int schedule_store_block(const block_msg_t *b)
{
    if (!b) return 0;
    if (b->block_index >= MAX_BLOCKS) return 0;

    uint8_t idx = b->block_index;
    g_blocks[idx] = *b;

    if (!g_block_valid[idx]) {
        g_block_valid[idx] = 1;
        g_sched_blocks_loaded++;
    }
    return 1;
}

void schedule_start(uint32_t now_ms)
{
    if (!g_block_valid[0]) {
        g_sched_running = 0;
        return;
    }

    g_sched_running = 1;
    g_sched_cur_index = 0;
    g_sched_block_start_ms = now_ms;

    apply_block(&g_blocks[g_sched_cur_index]);
}

void schedule_stop(void)
{
    g_sched_running = 0;
    all_off();
}

void schedule_tick(uint32_t now_ms)
{
    if (!g_sched_running) return;

    const block_msg_t *b = &g_blocks[g_sched_cur_index];

    if (!g_block_valid[g_sched_cur_index]) {
        schedule_stop();
        return;
    }

    uint32_t dur = b->duration_ms;
    if (dur == 0) dur = 1;

    if ((uint32_t)(now_ms - g_sched_block_start_ms) < dur) {
        return;
    }

    uint8_t next = (uint8_t)(g_sched_cur_index + 1);

    if (next >= MAX_BLOCKS || !g_block_valid[next]) {
        schedule_stop();
        return;
    }

    g_sched_cur_index = next;
    g_sched_block_start_ms = now_ms;
    apply_block(&g_blocks[g_sched_cur_index]);
}

static float clampf(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static float flow_to_vout(float flow_ulpm)
{
    const float dV = (V_MAX_VOLTS - V_MIN_VOLTS);
    const float dF = (FLOW_AT_VMAX_ULPM - FLOW_AT_VMIN_ULPM);

    if (dV <= 0.0f || dF <= 0.0f) {
        return V_MIN_VOLTS;
    }

    float v = V_MIN_VOLTS + (flow_ulpm - FLOW_AT_VMIN_ULPM) * (dV / dF);
    return clampf(v, V_MIN_VOLTS, V_MAX_VOLTS);
}

static void pump_set_flow(uint8_t pump_i, uint16_t flow_ulpm)
{
    float vout = 0.001232f * flow_ulpm - 0.034f;
    vout = clampf(vout, 0.034f, AVDD_VOLTS);
    writeDAC(pump_i, AVDD_VOLTS, vout, DAC_MODE_DEFAULT);
}

static void hw_stim_set(uint8_t on, uint16_t freq_x100, uint16_t duty_x10)
{
    (void)on; (void)freq_x100; (void)duty_x10;
}

