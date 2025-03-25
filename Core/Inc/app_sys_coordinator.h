/**
  ******************************************************************************
  * @file    app_sys_coordinator.h
  * @brief   Header for app_sys_coordinator.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_SYS_COORDINATOR_H
#define __APP_SYS_COORDINATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "app_comms_handler.h"
#include "app_led_driver.h"
#include "val_status.h"
#include "FreeRTOS.h"
#include "queue.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
    uint8_t lightId;    // Light source ID (1-3)
    uint8_t intensity;  // Current intensity (0-100)
} LightStatus_t;

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status SYS_Coordinator_Init(void);

/**
 * @brief Get the current intensity of a specific light source
 * @param lightId Light source ID (1-3)
 * @param intensity Pointer to store the retrieved intensity
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetLightIntensity(uint8_t lightId, uint8_t* intensity);

/**
 * @brief Get the current intensities of all light sources
 * @param intensities Array to store the retrieved intensities
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetAllLightIntensities(uint8_t* intensities);

/**
 * @brief Set the intensity for a specific light source
 * @param lightId Light source ID (1-3)
 * @param intensity Intensity value (0-100)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_SetLightIntensity(uint8_t lightId, uint8_t intensity);

/**
 * @brief Set intensities for all light sources
 * @param intensities Array of intensity values (0-100)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_SetAllLightIntensities(uint8_t* intensities);

/**
 * @brief Get sensor data for a specific light source
 * @param lightId Light source ID (1-3)
 * @param sensorData Pointer to store sensor readings
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetLightSensorData(uint8_t lightId, LightSensorData_t* sensorData);

/**
 * @brief Get sensor data for all light sources
 * @param sensorData Array to store sensor readings (must be size 3)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetAllLightSensorData(LightSensorData_t* sensorData);

/**
 * @brief Clear alarm for a specific light source
 * @param lightId Light source ID (1-3)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_ClearLightAlarm(uint8_t lightId);

#ifdef __cplusplus
}
#endif

#endif /* __APP_SYS_COORDINATOR_H */
