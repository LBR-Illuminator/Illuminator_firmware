/**
  ******************************************************************************
  * @file    val_sys_clock.h
  * @brief   Header for val_sys_clock.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_SYS_CLOCK_H
#define __VAL_SYS_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"
#include "stm32l4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_SysClock_Init(void);
uint32_t VAL_SysClock_GetTick(void);
VAL_Status VAL_SysClock_Delay(uint32_t delay);
uint32_t VAL_SysClock_GetFrequency(void);
uint32_t VAL_SysClock_GetMicros(void);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_SYS_CLOCK_H */
