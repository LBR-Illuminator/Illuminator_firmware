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
#include "val_status.h"
#include "FreeRTOS.h"
#include "queue.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status SYS_Coordinator_Init(void);
// QueueHandle_t getSystemCoordinatorCommandQueue(void);
// QueueHandle_t getSystemCoordinatorResponseQueue(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_SYS_COORDINATOR_H */
