#ifndef __PUMP_CARD_APP_H__
#define __PUMP_CARD_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

void pump_card_app_init(void);
void pump_card_app_poll(void);
void pump_card_app_on_sync_edge(void);

#ifdef __cplusplus
}
#endif

#endif /* __PUMP_CARD_APP_H__ */
