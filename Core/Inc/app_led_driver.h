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

/**
 * @brief Get the current intensity of a specific light source
 * @param lightId Light source ID (1-3)
 * @param intensity Pointer to store the retrieved intensity
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_GetIntensity(uint8_t lightId, uint8_t* intensity);

/**
 * @brief Get intensities for all light sources
 * @param intensities Array to store retrieved intensities
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_GetAllIntensities(uint8_t* intensities);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LED_DRIVER_H */
