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

#ifdef __cplusplus
}
#endif

#endif /* __APP_COMMS_HANDLER_H */
