#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void app_init(void);
void app_poll(void);
void app_on_cdc_rx(uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __APP_H__ */
