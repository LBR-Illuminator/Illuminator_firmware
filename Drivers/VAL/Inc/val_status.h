/**
  ******************************************************************************
  * @file    val_status.h
  * @brief   Header for status definitions in the Vendor Abstraction Layer
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_STATUS_H
#define __VAL_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef enum {
  VAL_OK = 0,       /* Operation completed successfully */
  VAL_ERROR,        /* General error */
  VAL_TIMEOUT,      /* Operation timed out */
  VAL_BUSY,         /* Resource is busy */
  VAL_PARAM         /* Invalid parameter */
} VAL_Status;

/* Error types for data store */
typedef enum {
  ERROR_OVER_CURRENT = 0,
  ERROR_OVER_TEMPERATURE = 1,
  ERROR_SYSTEM = 2
} ErrorType_t;

/* Status log structure */
typedef struct {
  uint8_t activeErrors;          /* Bitmap of lights with active errors (bits 0-2) */
  uint8_t errorTypes[3];         /* Error type for each light (0-2) */
  float errorValues[3];          /* Measured values that caused errors */
  uint32_t errorTimestamps[3];   /* Timestamps when errors occurred */
} StatusLog_t;

/* Error log entry */
typedef struct {
  uint32_t timestamp;            /* System time when error occurred (milliseconds) */
  uint8_t lightId;               /* Light source ID (1-3) */
  uint8_t errorType;             /* Type of error (using ErrorType_t) */
  float measuredValue;           /* The value that caused the error */
  uint8_t actionTaken;           /* Action taken (e.g., 1 = disabled light) */
} ErrorLogEntry_t;

#ifdef __cplusplus
}
#endif

#endif /* __VAL_STATUS_H */
