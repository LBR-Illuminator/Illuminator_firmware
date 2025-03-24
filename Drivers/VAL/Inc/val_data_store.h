/**
  ******************************************************************************
  * @file    val_data_store.h
  * @brief   Header for val_data_store.c module
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
#include "stm32l4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
  ERROR_OVER_CURRENT    = 0,
  ERROR_OVER_TEMPERATURE = 1,
  ERROR_SYSTEM          = 2
} ErrorType_t;

typedef struct {
  uint8_t activeErrors;      /* Bitmap of lights with active errors (bits 0-2) */
  uint8_t errorTypes[3];     /* Error type for each light (0-2) */
  float errorValues[3];      /* Measured values that caused errors */
  uint32_t errorTimestamps[3]; /* Timestamps when errors occurred */
} StatusLog_t;

typedef struct {
  uint32_t timestamp;        /* System time when error occurred (milliseconds) */
  uint8_t lightId;           /* Light source ID (1-3) */
  uint8_t errorType;         /* Type of error (using ErrorType_t) */
  float measuredValue;       /* The value that caused the error */
  uint8_t actionTaken;       /* Action taken (e.g., 1 = disabled light) */
} ErrorLogEntry_t;

typedef struct {
  uint16_t totalErrors;      /* Total number of errors recorded */
  uint8_t nextIndex;         /* Next position to write in circular buffer */
} ErrorLogHeader_t;

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_DataStore_Init(void);
VAL_Status VAL_DataStore_SetActiveError(uint8_t lightId, ErrorType_t errorType, float value);
VAL_Status VAL_DataStore_ClearActiveError(uint8_t lightId);
VAL_Status VAL_DataStore_HasActiveError(uint8_t lightId, uint8_t* hasError);
VAL_Status VAL_DataStore_GetStatusLog(StatusLog_t* status);
VAL_Status VAL_DataStore_LogErrorEvent(uint8_t lightId, ErrorType_t errorType, float value, uint8_t action);
VAL_Status VAL_DataStore_GetErrorLogs(ErrorLogEntry_t* logs, uint8_t maxLogs, uint8_t* count);
VAL_Status VAL_DataStore_ClearErrorLogs(void);
VAL_Status VAL_DataStore_DeInit(void);
VAL_Status VAL_DataStore_Format(void);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_DATA_STORE_H */
