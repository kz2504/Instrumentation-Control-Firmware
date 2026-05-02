#ifndef __SHIFTREG_H__
#define __SHIFTREG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void shiftByteEN(uint8_t byte);
void shiftByteDIR(uint8_t byte);
void clearEN(void);
void clearDIR(void);

#ifdef __cplusplus
}
#endif

#endif /* __SHIFTREG_H__ */
