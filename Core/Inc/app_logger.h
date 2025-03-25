/**
  ******************************************************************************
  * @file    app_logger.h
  * @brief   Header for app_logger.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_LOGGER_H
#define __APP_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status Logger_Init(void);
VAL_Status Logger_LogEvent(const char* event);
VAL_Status Logger_LogError(uint8_t severity, const char* error);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LOGGER_H */
