/**
  ******************************************************************************
  * @file    val_pwm.h
  * @brief   Header for val_pwm.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_PWM_H
#define __VAL_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_PWM_Init(void);
VAL_Status VAL_PWM_SetIntensity(uint8_t channel, uint8_t intensity);
VAL_Status VAL_PWM_GetIntensity(uint8_t channel, uint8_t* intensity);
VAL_Status VAL_PWM_SetAllIntensities(const uint8_t intensities[]);
VAL_Status VAL_PWM_StopChannel(uint8_t channel);
VAL_Status VAL_PWM_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_PWM_H */
