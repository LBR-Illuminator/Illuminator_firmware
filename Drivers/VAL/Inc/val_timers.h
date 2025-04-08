/**
  ******************************************************************************
  * @file    val_timers.h
  * @brief   Header for val_timers.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_TIMERS_H
#define __VAL_TIMERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"
#include "stm32l4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_Timers_Init(void);
VAL_Status VAL_Timers_DeInit(void);

/* Timer callback declarations moved from main.c */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_TIMERS_H */
