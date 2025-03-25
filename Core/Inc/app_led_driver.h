/**
  ******************************************************************************
  * @file    app_led_driver.h
  * @brief   Header for app_led_driver.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_LED_DRIVER_H
#define __APP_LED_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status LED_Driver_Init(void);
VAL_Status LED_Driver_SetIntensity(uint8_t lightId, uint8_t intensity);
VAL_Status LED_Driver_SetAllIntensities(uint8_t *intensities);
VAL_Status LED_Driver_ClearAlarm(uint8_t lightId);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LED_DRIVER_H */
