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
  ERROR_OVER_CURRENT = 1,
  ERROR_OVER_TEMPERATURE = 2,
  ERROR_SYSTEM = 3
} ErrorType_t;

/* Status log structure */
typedef struct {
  uint8_t active_errors;          /* Bitmap of lights with active errors (bits 0-2) */
  uint8_t error_types[3];         /* Error type for each light (0-2) */
  float error_values[3];          /* Measured values that caused errors */
  uint32_t error_timestamps[3];   /* Timestamps when errors occurred */
} StatusLog_t;

/* Error log entry */
typedef struct {
  uint32_t timestamp;            /* System time when error occurred (milliseconds) */
  uint8_t light_id;               /* Light source ID (1-3) */
  uint8_t error_type;             /* Type of error (using ErrorType_t) */
  float measured_value;           /* The value that caused the error */
  uint8_t action_taken;           /* Action taken (e.g., 1 = disabled light) */
} ErrorLogEntry_t;

#ifdef __cplusplus
}
#endif

#endif /* __VAL_STATUS_H */
