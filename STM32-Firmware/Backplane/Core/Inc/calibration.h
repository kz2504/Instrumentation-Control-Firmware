#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PUMP_CAL_COUNT 64U

typedef struct
{
  int32_t m_q16_16;
  int32_t b_q16_16;
} PumpCalibration;

extern PumpCalibration pump_cal[PUMP_CAL_COUNT];

void calibration_init(void);
uint8_t calibration_run_pump_from_payload(const uint8_t *payload, uint16_t payload_len, uint8_t *detail);
uint8_t calibration_set_coeff_from_payload(const uint8_t *payload, uint16_t payload_len, uint8_t *detail);
void calibration_run_pump_stub(uint8_t pump_id, uint8_t direction, uint32_t voltage_mV, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif /* __CALIBRATION_H__ */
