/**
  ******************************************************************************
  * @file    app_comms_handler.h
  * @brief   Header for app_comms_handler.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_COMMS_HANDLER_H
#define __APP_COMMS_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status COMMS_Handler_Init(void);

/**
 * @brief Send alarm event notification
 * @param lightId Light source ID that triggered the alarm (1-3)
 * @param errorType Type of error that occurred
 * @param value Measured value that caused the alarm
 * @retval VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status COMMS_SendAlarmEvent(uint8_t lightId, uint8_t errorType, float value);

#ifdef __cplusplus
}
#endif

#endif /* __APP_COMMS_HANDLER_H */
