/**
  ******************************************************************************
  * @file    val_analog.h
  * @brief   Header for val_analog.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_ANALOG_H
#define __VAL_ANALOG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
  uint8_t lightId;       /* Light ID (1-3) */
  float current;         /* Current in milliamps */
  float temperature;     /* Temperature in degrees Celsius */
} LightSensorData;

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_Analog_Init(void);
VAL_Status VAL_Analog_GetCurrent(uint8_t lightId, float* current);
VAL_Status VAL_Analog_GetTemperature(uint8_t lightId, float* temperature);
VAL_Status VAL_Analog_GetSensorData(uint8_t lightId, LightSensorData* sensorData);
VAL_Status VAL_Analog_GetAllSensorData(LightSensorData sensorData[]);
VAL_Status VAL_Analog_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_ANALOG_H */
