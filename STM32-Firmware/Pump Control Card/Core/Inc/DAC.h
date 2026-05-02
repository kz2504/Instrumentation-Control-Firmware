#ifndef __DAC_H__
#define __DAC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void dac_setCS(uint8_t line);
void dac_resetCS(void);
void writeDAC(uint8_t channel, float AVDD, float vOut, uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __DAC_H__ */
