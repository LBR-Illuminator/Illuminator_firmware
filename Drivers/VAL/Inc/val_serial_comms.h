/**
  ******************************************************************************
  * @file    val_serial_comms.h
  * @brief   Header for val_serial_comms.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_SERIAL_COMMS_H
#define __VAL_SERIAL_COMMS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*SerialRxCallback)(uint8_t byte);

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_Serial_Init(SerialRxCallback callback);
VAL_Status VAL_Serial_Send(const uint8_t* data, uint16_t length, uint32_t timeout);
VAL_Status VAL_Serial_Printf(const char* format, ...);
uint8_t VAL_Serial_IsBusy(void);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_SERIAL_COMMS_H */
