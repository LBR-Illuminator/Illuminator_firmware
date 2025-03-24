/**
  ******************************************************************************
  * @file    val_pins.h
  * @brief   Header for val_pins.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_PINS_H
#define __VAL_PINS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "val_status.h"

/* Exported functions prototypes ---------------------------------------------*/
VAL_Status VAL_Pins_Init(void);
VAL_Status VAL_Pins_SetBoardLED(uint8_t state);
VAL_Status VAL_Pins_ToggleBoardLED(void);
VAL_Status VAL_Pins_GetBoardLED(uint8_t* state);
VAL_Status VAL_Pins_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __VAL_PINS_H */
