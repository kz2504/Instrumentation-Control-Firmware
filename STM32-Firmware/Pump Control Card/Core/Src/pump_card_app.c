#include "pump_card_app.h"

#include "DAC.h"
#include "main.h"
#include "pump_card_protocol.h"
#include "shiftreg.h"
#include "spi.h"

#include <string.h>

#define PUMP_CARD_LED_BLINK_MS    5U
#define PUMP_CARD_DAC_AVDD_V      5.0f
#define PUMP_CARD_DAC_ZERO_V      0.0f
#define PUMP_CARD_MAX_VOLTAGE_MV  5000U

typedef struct
{
  PumpCardEvent events[PUMP_CARD_MAX_LOCAL_EVENTS];
  uint16_t event_count;
  uint16_t next_event_index;
  uint32_t last_global_event_id;
  uint8_t armed;
  uint8_t en_bits;
  uint8_t dir_bits;
  uint8_t blink_pending;
} PumpCardStore;

static PumpCardStore g_pump_card;
static uint8_t g_rx_frame[PUMP_CARD_CMD_FRAME_SIZE];
static uint8_t g_tx_frame[PUMP_CARD_RESP_FRAME_SIZE];

static void pump_card_set_led(uint8_t on)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static uint32_t pump_card_flow_to_voltage_mV(uint32_t flow_ul_min)
{
  uint64_t voltage_mV;

  voltage_mV = (uint64_t)flow_ul_min * 5ULL;
  if (voltage_mV > PUMP_CARD_MAX_VOLTAGE_MV)
  {
    voltage_mV = PUMP_CARD_MAX_VOLTAGE_MV;
  }

  return (uint32_t)voltage_mV;
}

static float pump_card_mV_to_volts(uint32_t voltage_mV)
{
  return (float)voltage_mV / 1000.0f;
}

static void pump_card_disarm(void)
{
  g_pump_card.armed = 0U;
  g_pump_card.next_event_index = 0U;
}

static void pump_card_set_enable_bit(uint8_t local_pump_id, uint8_t enable)
{
  uint8_t mask = (uint8_t)(1U << local_pump_id);

  if (enable != 0U)
  {
    g_pump_card.en_bits |= mask;
  }
  else
  {
    g_pump_card.en_bits &= (uint8_t)(~mask);
  }
}

static void pump_card_set_direction_bit(uint8_t local_pump_id, uint8_t direction)
{
  uint8_t mask = (uint8_t)(1U << local_pump_id);

  if (direction != 0U)
  {
    g_pump_card.dir_bits |= mask;
  }
  else
  {
    g_pump_card.dir_bits &= (uint8_t)(~mask);
  }
}

static void pump_card_outputs_all_off(void)
{
  uint8_t channel;

  g_pump_card.en_bits = 0U;
  g_pump_card.dir_bits = 0U;
  clearEN();
  clearDIR();

  for (channel = 0U; channel < PUMP_CARD_PUMPS_PER_CARD; channel++)
  {
    writeDAC(channel, PUMP_CARD_DAC_AVDD_V, PUMP_CARD_DAC_ZERO_V, 0U);
  }
}

static void pump_card_reset_schedule(void)
{
  memset(g_pump_card.events, 0, sizeof(g_pump_card.events));
  g_pump_card.event_count = 0U;
  g_pump_card.last_global_event_id = 0U;
  g_pump_card.blink_pending = 0U;
  pump_card_disarm();
}

static uint8_t pump_card_validate_action(const PumpAction *action)
{
  if (action->local_pump_id >= PUMP_CARD_PUMPS_PER_CARD)
  {
    return PUMP_STATUS_BAD_INDEX;
  }

  if ((action->enable > 1U) || (action->direction > 1U))
  {
    return PUMP_STATUS_BAD_LEN;
  }

  return PUMP_STATUS_OK;
}

static uint8_t pump_apply_action(const PumpAction *action)
{
  uint32_t voltage_mV;
  uint8_t validation_status;

  if (action == 0)
  {
    return PUMP_STATUS_BAD_LEN;
  }

  validation_status = pump_card_validate_action(action);
  if (validation_status != PUMP_STATUS_OK)
  {
    return validation_status;
  }

  if (action->enable != 0U)
  {
    pump_card_set_direction_bit(action->local_pump_id, action->direction);
    shiftByteDIR(g_pump_card.dir_bits);

    voltage_mV = pump_card_flow_to_voltage_mV(action->flow_ul_min);
    writeDAC(action->local_pump_id,
             PUMP_CARD_DAC_AVDD_V,
             pump_card_mV_to_volts(voltage_mV),
             0U);

    pump_card_set_enable_bit(action->local_pump_id, 1U);
    shiftByteEN(g_pump_card.en_bits);
  }
  else
  {
    pump_card_set_enable_bit(action->local_pump_id, 0U);
    shiftByteEN(g_pump_card.en_bits);

    writeDAC(action->local_pump_id, PUMP_CARD_DAC_AVDD_V, PUMP_CARD_DAC_ZERO_V, 0U);

    pump_card_set_direction_bit(action->local_pump_id, action->direction);
    shiftByteDIR(g_pump_card.dir_bits);
  }

  return PUMP_STATUS_OK;
}

static uint8_t pump_apply_event(const PumpCardEvent *event)
{
  uint8_t action_index;
  uint8_t validation_status;

  if (event == 0)
  {
    return PUMP_STATUS_BAD_LEN;
  }

  for (action_index = 0U; action_index < event->action_count; action_index++)
  {
    validation_status = pump_apply_action(&event->actions[action_index]);
    if (validation_status != PUMP_STATUS_OK)
    {
      return validation_status;
    }
  }

  return PUMP_STATUS_OK;
}

static void pump_card_build_response(uint8_t ack,
                                     uint8_t status,
                                     const uint8_t *payload,
                                     uint16_t payload_len)
{
  if (!pump_card_encode_response_frame(ack,
                                       status,
                                       payload,
                                       payload_len,
                                       g_tx_frame,
                                       sizeof(g_tx_frame)))
  {
    memset(g_tx_frame, 0, sizeof(g_tx_frame));
  }
}

static void pump_card_build_ok_response(void)
{
  pump_card_build_response(PUMP_ACK_OK, PUMP_STATUS_OK, 0, 0U);
}

static void pump_card_build_error_response(uint8_t status)
{
  pump_card_build_response(PUMP_ACK_ERR, status, 0, 0U);
}

static void pump_card_handle_command(const PumpCardCommand *command)
{
  PumpCardEvent event = {0};
  PumpCardIdentifyInfo identify = {0};
  PumpCardStatusInfo status = {0};
  uint8_t payload[PUMP_CARD_RESP_PAYLOAD_MAX] = {0};
  uint16_t payload_len = 0U;
  uint8_t action_index;
  uint16_t exec_index;
  uint32_t exec_global_event_id;
  uint8_t validation_status;

  switch (command->command_id)
  {
    case CMD_IDENTIFY:
      identify.card_type = PUMP_CARD_TYPE_PERISTALTIC;
      identify.firmware_major = PUMP_CARD_FW_MAJOR;
      identify.firmware_minor = PUMP_CARD_FW_MINOR;
      identify.capabilities = PUMP_CARD_CAP_PERISTALTIC;
      identify.max_local_events = PUMP_CARD_MAX_LOCAL_EVENTS;
      pump_card_build_identify_payload(&identify, payload, &payload_len);
      pump_card_build_response(PUMP_ACK_OK, PUMP_STATUS_OK, payload, payload_len);
      break;

    case CMD_CLEAR:
      pump_card_outputs_all_off();
      pump_card_reset_schedule();
      pump_card_build_ok_response();
      break;

    case CMD_LOAD_EVENT:
      if (!pump_card_parse_load_event_payload(command->payload, command->payload_len, &event))
      {
        pump_card_build_error_response(PUMP_STATUS_BAD_LEN);
        break;
      }

      if (event.local_event_index >= PUMP_CARD_MAX_LOCAL_EVENTS)
      {
        pump_card_build_error_response(PUMP_STATUS_FULL);
        break;
      }

      if (event.local_event_index > g_pump_card.event_count)
      {
        pump_card_build_error_response(PUMP_STATUS_BAD_INDEX);
        break;
      }

      for (action_index = 0U; action_index < event.action_count; action_index++)
      {
        validation_status = pump_card_validate_action(&event.actions[action_index]);
        if (validation_status != PUMP_STATUS_OK)
        {
          pump_card_build_error_response(validation_status);
          return;
        }
      }

      g_pump_card.events[event.local_event_index] = event;
      if (event.local_event_index == g_pump_card.event_count)
      {
        g_pump_card.event_count++;
      }

      pump_card_disarm();
      pump_card_build_ok_response();
      break;

    case CMD_ARM:
      g_pump_card.armed = 1U;
      g_pump_card.next_event_index = 0U;
      pump_card_build_ok_response();
      break;

    case CMD_EXEC_EVENT:
      if (!pump_card_parse_exec_event_payload(command->payload,
                                              command->payload_len,
                                              &exec_index,
                                              &exec_global_event_id))
      {
        pump_card_build_error_response(PUMP_STATUS_BAD_LEN);
        break;
      }

      if (g_pump_card.armed == 0U)
      {
        pump_card_build_error_response(PUMP_STATUS_NOT_ARMED);
        break;
      }

      if ((exec_index != g_pump_card.next_event_index) || (exec_index >= g_pump_card.event_count))
      {
        pump_card_build_error_response(PUMP_STATUS_BAD_INDEX);
        break;
      }

      if (g_pump_card.events[exec_index].global_event_id != exec_global_event_id)
      {
        pump_card_build_error_response(PUMP_STATUS_BAD_INDEX);
        break;
      }

      validation_status = pump_apply_event(&g_pump_card.events[exec_index]);
      if (validation_status != PUMP_STATUS_OK)
      {
        pump_card_build_error_response(validation_status);
        break;
      }

      pump_card_set_led(1U);
      g_pump_card.blink_pending = 1U;
      g_pump_card.last_global_event_id = exec_global_event_id;
      g_pump_card.next_event_index++;
      pump_card_build_ok_response();
      break;

    case CMD_STOP:
      pump_card_disarm();
      pump_card_outputs_all_off();
      pump_card_build_ok_response();
      break;

    case CMD_STATUS:
      status.armed = g_pump_card.armed;
      status.event_count = g_pump_card.event_count;
      status.next_event_index = g_pump_card.next_event_index;
      status.last_global_event_id = g_pump_card.last_global_event_id;
      pump_card_build_status_payload(&status, payload, &payload_len);
      pump_card_build_response(PUMP_ACK_OK, PUMP_STATUS_OK, payload, payload_len);
      break;

    default:
      pump_card_build_error_response(PUMP_STATUS_BAD_CMD);
      break;
  }
}

void pump_card_app_init(void)
{
  memset(&g_pump_card, 0, sizeof(g_pump_card));
  memset(g_rx_frame, 0, sizeof(g_rx_frame));
  memset(g_tx_frame, 0, sizeof(g_tx_frame));
  pump_card_set_led(0U);
  pump_card_reset_schedule();
  pump_card_outputs_all_off();
  pump_card_build_error_response(PUMP_STATUS_BAD_CMD);
}

void pump_card_app_poll(void)
{
  PumpCardCommand command;
  uint8_t frame_status;

  memset(&command, 0, sizeof(command));

  if (HAL_SPI_Receive(&hspi1, g_rx_frame, sizeof(g_rx_frame), HAL_MAX_DELAY) != HAL_OK)
  {
    return;
  }

  frame_status = pump_card_decode_command_frame(g_rx_frame, sizeof(g_rx_frame), &command);
  if (frame_status != PUMP_STATUS_OK)
  {
    pump_card_build_error_response(frame_status);
  }
  else
  {
    pump_card_handle_command(&command);
  }

  (void)HAL_SPI_Transmit(&hspi1, g_tx_frame, sizeof(g_tx_frame), HAL_MAX_DELAY);

  if (g_pump_card.blink_pending != 0U)
  {
    HAL_Delay(PUMP_CARD_LED_BLINK_MS);
    pump_card_set_led(0U);
    g_pump_card.blink_pending = 0U;
  }
}
