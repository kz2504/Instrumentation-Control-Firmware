#include "calibration.h"

#include "protocol.h"

#include <string.h>

#define CAL_RUN_PUMP_PAYLOAD_SIZE    12U
#define CAL_SET_COEFF_PAYLOAD_SIZE   12U

PumpCalibration pump_cal[PUMP_CAL_COUNT];

void calibration_init(void)
{
  memset(pump_cal, 0, sizeof(pump_cal));
}

uint8_t calibration_run_pump_from_payload(const uint8_t *payload, uint16_t payload_len, uint8_t *detail)
{
  uint8_t pump_id;
  uint8_t direction;
  uint32_t voltage_mV;
  uint32_t duration_ms;

  if (detail != 0)
  {
    *detail = 0U;
  }

  if ((payload == 0) || (payload_len != CAL_RUN_PUMP_PAYLOAD_SIZE))
  {
    return ERR_BAD_EVENT;
  }

  pump_id = payload[0];
  direction = payload[1];
  voltage_mV = read_u32_le(&payload[4]);
  duration_ms = read_u32_le(&payload[8]);

  if (pump_id >= PUMP_CAL_COUNT)
  {
    if (detail != 0)
    {
      *detail = pump_id;
    }
    return ERR_BAD_MODULE;
  }

  if (direction > 1U)
  {
    if (detail != 0)
    {
      *detail = direction;
    }
    return ERR_BAD_ACTION;
  }

  calibration_run_pump_stub(pump_id, direction, voltage_mV, duration_ms);
  return ACK_OK;
}

uint8_t calibration_set_coeff_from_payload(const uint8_t *payload, uint16_t payload_len, uint8_t *detail)
{
  uint8_t pump_id;

  if (detail != 0)
  {
    *detail = 0U;
  }

  if ((payload == 0) || (payload_len != CAL_SET_COEFF_PAYLOAD_SIZE))
  {
    return ERR_BAD_EVENT;
  }

  pump_id = payload[0];
  if (pump_id >= PUMP_CAL_COUNT)
  {
    if (detail != 0)
    {
      *detail = pump_id;
    }
    return ERR_BAD_MODULE;
  }

  pump_cal[pump_id].m_q16_16 = read_i32_le(&payload[4]);
  pump_cal[pump_id].b_q16_16 = read_i32_le(&payload[8]);

  return ACK_OK;
}

void calibration_run_pump_stub(uint8_t pump_id, uint8_t direction, uint32_t voltage_mV, uint32_t duration_ms)
{
  (void)pump_id;
  (void)direction;
  (void)voltage_mV;
  (void)duration_ms;
}
