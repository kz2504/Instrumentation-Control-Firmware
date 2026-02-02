#pragma once
#include <stdint.h>

#ifndef NUM_PUMPS
#define NUM_PUMPS 8
#endif

#ifndef MAX_BLOCKS
#define MAX_BLOCKS 64
#endif

typedef struct __attribute__((packed)) {
    uint8_t  block_index;
    uint32_t duration_ms;

    uint8_t state_mask; //Bit i: pump i enable
    uint8_t dir_mask; //Bit i: pump i direction

    uint16_t flow[NUM_PUMPS];

    uint8_t  stim_on;
    uint16_t stim_freq_x100;
    uint16_t stim_duty_x10;
} block_msg_t;

//Debug watch vars
extern volatile uint8_t  g_sched_running;
extern volatile uint8_t  g_sched_cur_index;
extern volatile uint32_t g_sched_block_start_ms;
extern volatile uint32_t g_sched_blocks_loaded;

void schedule_clear(void);
int  schedule_store_block(const block_msg_t *b);
void schedule_start(uint32_t now_ms);
void schedule_stop(void);
void schedule_tick(uint32_t now_ms);
