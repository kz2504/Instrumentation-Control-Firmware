#include "pump_card_regs.h"

#include "DAC.h"
#include "card_registers.h"
#include "shiftreg.h"
#include "spi_frame.h"

#include <string.h>

#define PUMP_CARD_PUMPS_PER_CARD  8U
#define PUMP_CARD_DAC_AVDD_V      5.0f
#define PUMP_CARD_DAC_ZERO_V      0.0f
#define PUMP_CARD_MAX_VOLTAGE_MV  5000U

typedef struct
{
  uint32_t control;
  uint32_t speed_mV;
} PumpOutputState;

typedef struct
{
  uint32_t error_flags;
  uint32_t control;
  uint8_t en_bits;
  uint8_t dir_bits;
  uint32_t stage_event_index;
  uint32_t stage_meta;
  uint32_t stage_speed_mV;
  uint16_t queue_count;
  uint16_t queue_head;
  uint32_t current_event_index;
  uint32_t last_event_index;
  uint8_t queue_push_seen;
  uint8_t run_active;
  uint8_t run_done;
  uint32_t queue_event_index[PUMP_CARD_MAX_LOCAL_EVENTS];
  uint32_t queue_meta[PUMP_CARD_MAX_LOCAL_EVENTS];
  uint32_t queue_speed_mV[PUMP_CARD_MAX_LOCAL_EVENTS];
  PumpOutputState outputs[PUMP_CARD_PUMPS_PER_CARD];
} PumpCardRegsContext;

static PumpCardRegsContext g_pump_regs;

static void pump_card_set_enable_bit(uint8_t local_pump_id, uint8_t enable)
{
  uint8_t mask = (uint8_t)(1U << local_pump_id);

  if (enable != 0U)
  {
    g_pump_regs.en_bits |= mask;
  }
  else
  {
    g_pump_regs.en_bits &= (uint8_t)(~mask);
  }
}

static void pump_card_set_direction_bit(uint8_t local_pump_id, uint8_t direction)
{
  uint8_t mask = (uint8_t)(1U << local_pump_id);

  if (direction != 0U)
  {
    g_pump_regs.dir_bits |= mask;
  }
  else
  {
    g_pump_regs.dir_bits &= (uint8_t)(~mask);
  }
}

static void pump_card_apply_outputs(uint8_t index)
{
  uint32_t speed_mV = g_pump_regs.outputs[index].speed_mV;
  uint32_t control = g_pump_regs.outputs[index].control;
  uint8_t enable = (uint8_t)(control & PUMP_CONTROL_ENABLE);
  uint8_t direction = (uint8_t)((control & PUMP_CONTROL_DIRECTION) != 0U);

  if (speed_mV > PUMP_CARD_MAX_VOLTAGE_MV)
  {
    speed_mV = PUMP_CARD_MAX_VOLTAGE_MV;
    g_pump_regs.outputs[index].speed_mV = speed_mV;
  }

  if (enable != 0U)
  {
    pump_card_set_direction_bit(index, direction);
    shiftByteDIR(g_pump_regs.dir_bits);
    writeDAC(index, PUMP_CARD_DAC_AVDD_V, (float)speed_mV / 1000.0f, 0U);
    pump_card_set_enable_bit(index, 1U);
    shiftByteEN(g_pump_regs.en_bits);
  }
  else
  {
    pump_card_set_enable_bit(index, 0U);
    shiftByteEN(g_pump_regs.en_bits);
    writeDAC(index, PUMP_CARD_DAC_AVDD_V, PUMP_CARD_DAC_ZERO_V, 0U);
    pump_card_set_direction_bit(index, direction);
    shiftByteDIR(g_pump_regs.dir_bits);
  }
}

static void pump_card_set_output_state(uint8_t index, uint32_t control, uint32_t speed_mV)
{
  g_pump_regs.outputs[index].control = (control & (PUMP_CONTROL_ENABLE | PUMP_CONTROL_DIRECTION));
  g_pump_regs.outputs[index].speed_mV = speed_mV;

  if ((g_pump_regs.outputs[index].control & PUMP_CONTROL_ENABLE) == 0U)
  {
    g_pump_regs.outputs[index].speed_mV = 0U;
  }

  pump_card_apply_outputs(index);
}

static void pump_card_outputs_all_off(void)
{
  uint8_t pump_index;

  g_pump_regs.en_bits = 0U;
  g_pump_regs.dir_bits = 0U;
  clearEN();
  clearDIR();

  for (pump_index = 0U; pump_index < PUMP_CARD_PUMPS_PER_CARD; pump_index++)
  {
    writeDAC(pump_index, PUMP_CARD_DAC_AVDD_V, PUMP_CARD_DAC_ZERO_V, 0U);
    g_pump_regs.outputs[pump_index].control = 0U;
    g_pump_regs.outputs[pump_index].speed_mV = 0U;
  }
}

static void pump_card_reset_queue_state(void)
{
  g_pump_regs.control = 0U;
  g_pump_regs.stage_event_index = 0U;
  g_pump_regs.stage_meta = 0U;
  g_pump_regs.stage_speed_mV = 0U;
  g_pump_regs.queue_count = 0U;
  g_pump_regs.queue_head = 0U;
  g_pump_regs.current_event_index = 0U;
  g_pump_regs.last_event_index = 0U;
  g_pump_regs.queue_push_seen = 0U;
  g_pump_regs.run_active = 0U;
  g_pump_regs.run_done = 0U;
  pump_card_outputs_all_off();
}

static bool pump_card_queue_write_allowed(uint8_t *status_out)
{
  if (g_pump_regs.run_active != 0U)
  {
    *status_out = STATUS_BAD_CMD;
    return false;
  }

  return true;
}

static bool pump_card_validate_staged_entry(uint8_t *status_out)
{
  uint8_t local_pump = PUMP_QUEUE_META_LOCAL_PUMP(g_pump_regs.stage_meta);
  uint32_t control = PUMP_QUEUE_META_CONTROL(g_pump_regs.stage_meta);

  if (local_pump >= PUMP_CARD_PUMPS_PER_CARD)
  {
    *status_out = STATUS_BAD_ADDR;
    return false;
  }

  if ((control & ~(PUMP_CONTROL_ENABLE | PUMP_CONTROL_DIRECTION)) != 0U)
  {
    *status_out = STATUS_BAD_CMD;
    return false;
  }

  return true;
}

void pump_card_regs_init(void)
{
  memset(&g_pump_regs, 0, sizeof(g_pump_regs));
  pump_card_reset_queue_state();
}

bool pump_card_regs_read(uint16_t reg_addr, uint32_t *value_out, uint8_t *status_out)
{
  if ((value_out == 0) || (status_out == 0))
  {
    return false;
  }

  *status_out = STATUS_OK;

  switch (reg_addr)
  {
    case CARD_REG_ID:
      *value_out = CARD_ID_MAGIC;
      return true;

    case CARD_REG_TYPE:
      *value_out = CARD_TYPE_PUMP_PERISTALTIC;
      return true;

    case CARD_REG_FW_VERSION:
      *value_out = CARD_FW_VERSION(PUMP_CARD_FW_MAJOR, PUMP_CARD_FW_MINOR);
      return true;

    case CARD_REG_STATUS:
      *value_out = 0U;
      if (g_pump_regs.run_active != 0U)
      {
        *value_out |= PUMP_CARD_STATUS_RUNNING;
      }
      if (g_pump_regs.run_done != 0U)
      {
        *value_out |= PUMP_CARD_STATUS_DONE;
      }
      if (g_pump_regs.queue_count != 0U)
      {
        *value_out |= PUMP_CARD_STATUS_QUEUE_READY;
      }
      return true;

    case CARD_REG_ERROR_FLAGS:
      *value_out = g_pump_regs.error_flags;
      return true;

    case CARD_REG_CONTROL:
      *value_out = g_pump_regs.control;
      return true;

    case CARD_REG_CAPABILITIES:
      *value_out = CARD_CAP_PUMP_PERISTALTIC;
      return true;

    case CARD_REG_MAX_LOCAL_EVENTS:
      *value_out = PUMP_CARD_MAX_LOCAL_EVENTS;
      return true;

    case PUMP_QUEUE_REG_EVENT_INDEX:
      *value_out = g_pump_regs.stage_event_index;
      return true;

    case PUMP_QUEUE_REG_META:
      *value_out = g_pump_regs.stage_meta;
      return true;

    case PUMP_QUEUE_REG_SPEED_MV:
      *value_out = g_pump_regs.stage_speed_mV;
      return true;

    case PUMP_QUEUE_REG_COUNT:
      *value_out = g_pump_regs.queue_count;
      return true;

    case PUMP_QUEUE_REG_STATUS:
      *value_out = 0U;
      if (g_pump_regs.queue_count >= PUMP_CARD_MAX_LOCAL_EVENTS)
      {
        *value_out |= PUMP_QUEUE_STATUS_FULL;
      }
      if (g_pump_regs.queue_push_seen != 0U)
      {
        *value_out |= PUMP_QUEUE_STATUS_PUSH_SEEN;
      }
      if (g_pump_regs.run_active != 0U)
      {
        *value_out |= PUMP_QUEUE_STATUS_RUNNING;
      }
      if (g_pump_regs.run_done != 0U)
      {
        *value_out |= PUMP_QUEUE_STATUS_DONE;
      }
      return true;

    case PUMP_QUEUE_REG_LAST_EVENT_INDEX:
      *value_out = g_pump_regs.last_event_index;
      return true;

    default:
      *status_out = STATUS_BAD_ADDR;
      return false;
  }
}

bool pump_card_regs_write(uint16_t reg_addr, uint32_t value, uint8_t *status_out)
{
  if (status_out == 0)
  {
    return false;
  }

  *status_out = STATUS_OK;

  switch (reg_addr)
  {
    case CARD_REG_ERROR_FLAGS:
      g_pump_regs.error_flags = value;
      return true;

    case CARD_REG_CONTROL:
      if ((value & PUMP_CARD_CONTROL_CLEAR_QUEUE) != 0U)
      {
        pump_card_reset_queue_state();
      }

      if ((value & PUMP_CARD_CONTROL_RUN) != 0U)
      {
        g_pump_regs.control = PUMP_CARD_CONTROL_RUN;
        g_pump_regs.queue_head = 0U;
        g_pump_regs.current_event_index = 0U;
        g_pump_regs.run_active = (g_pump_regs.queue_count != 0U) ? 1U : 0U;
        g_pump_regs.run_done = (g_pump_regs.queue_count == 0U) ? 1U : 0U;
      }
      else if ((value & PUMP_CARD_CONTROL_CLEAR_QUEUE) == 0U)
      {
        g_pump_regs.control = 0U;
        g_pump_regs.run_active = 0U;
      }
      return true;

    case PUMP_QUEUE_REG_EVENT_INDEX:
      if (!pump_card_queue_write_allowed(status_out))
      {
        return false;
      }

      g_pump_regs.stage_event_index = value;
      return true;

    case PUMP_QUEUE_REG_META:
      if (!pump_card_queue_write_allowed(status_out))
      {
        return false;
      }

      g_pump_regs.stage_meta = value;
      return true;

    case PUMP_QUEUE_REG_SPEED_MV:
      if (!pump_card_queue_write_allowed(status_out))
      {
        return false;
      }

      g_pump_regs.stage_speed_mV = value;
      return true;

    case PUMP_QUEUE_REG_PUSH:
      if (!pump_card_queue_write_allowed(status_out))
      {
        return false;
      }

      if (g_pump_regs.queue_count >= PUMP_CARD_MAX_LOCAL_EVENTS)
      {
        *status_out = STATUS_BAD_LEN;
        return false;
      }

      if (!pump_card_validate_staged_entry(status_out))
      {
        return false;
      }

      g_pump_regs.queue_event_index[g_pump_regs.queue_count] = g_pump_regs.stage_event_index;
      g_pump_regs.queue_meta[g_pump_regs.queue_count] = g_pump_regs.stage_meta;
      g_pump_regs.queue_speed_mV[g_pump_regs.queue_count] = g_pump_regs.stage_speed_mV;
      g_pump_regs.queue_count++;
      g_pump_regs.last_event_index = g_pump_regs.stage_event_index;
      g_pump_regs.queue_push_seen = 1U;
      g_pump_regs.run_done = 0U;
      return true;

    default:
      *status_out = STATUS_BAD_ADDR;
      return false;
  }
}

void pump_card_regs_on_sync_edge(void)
{
  while ((g_pump_regs.run_active != 0U) &&
         (g_pump_regs.queue_head < g_pump_regs.queue_count) &&
         (g_pump_regs.queue_event_index[g_pump_regs.queue_head] == g_pump_regs.current_event_index))
  {
    uint8_t local_pump = PUMP_QUEUE_META_LOCAL_PUMP(g_pump_regs.queue_meta[g_pump_regs.queue_head]);
    uint32_t control = PUMP_QUEUE_META_CONTROL(g_pump_regs.queue_meta[g_pump_regs.queue_head]);
    uint32_t speed_mV = g_pump_regs.queue_speed_mV[g_pump_regs.queue_head];

    if (local_pump < PUMP_CARD_PUMPS_PER_CARD)
    {
      pump_card_set_output_state(local_pump, control, speed_mV);
    }

    g_pump_regs.queue_head++;
  }

  if (g_pump_regs.run_active != 0U)
  {
    g_pump_regs.current_event_index++;
    if (g_pump_regs.queue_head >= g_pump_regs.queue_count)
    {
      g_pump_regs.control = 0U;
      g_pump_regs.run_active = 0U;
      g_pump_regs.run_done = 1U;
    }
  }
}
