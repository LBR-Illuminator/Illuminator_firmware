/**
  ******************************************************************************
  * @file    val_data_store.h
  * @brief   Header for data storage in the Vendor Abstraction Layer
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_DATA_STORE_H
#define __VAL_DATA_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_DataStore_Init(void);
uint16_t VAL_DataStore_GetErrorCount(void);
uint8_t VAL_DataStore_GetErrorLogs(ErrorLogEntry_t *logs, uint8_t maxCount);
VAL_Status VAL_DataStore_ClearErrorLogs(void);
VAL_Status VAL_DataStore_GetStatusLog(StatusLog_t *statusLog);
VAL_Status VAL_DataStore_SetActiveError(uint8_t lightId, ErrorType_t errorType, float value);
VAL_Status VAL_DataStore_ClearActiveError(uint8_t lightId);
VAL_Status VAL_DataStore_LogErrorEvent(uint8_t lightId, ErrorType_t errorType, float value, uint8_t action);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_DATA_STORE_H */
