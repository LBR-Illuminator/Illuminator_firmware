/**
  ******************************************************************************
  * @file    val.h
  * @brief   Vendor Abstraction Layer main header
  ******************************************************************************
  * @attention
  *
  * This file includes all the VAL module headers for convenient import.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_H
#define __VAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "val_status.h"
#include "val_serial_comms.h"
#include "val_pwm.h"
#include "val_analog.h"
#include "val_data_store.h"
#include "val_pins.h"
#include "val_sys_clock.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Initialize all VAL modules
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
static inline VAL_Status VAL_Init(void) {
  VAL_Status status;
  
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Initialize system clock first */
  status = VAL_SysClock_Init();
  if (status != VAL_OK) {
    return status;
  }
  
  /* Initialize GPIO pins */
  status = VAL_Pins_Init();
  if (status != VAL_OK) {
    return status;
  }

  /* Initialize PWM */
  status = VAL_PWM_Init();
  if (status != VAL_OK) {
    return status;
  }
  
  /* Initialize analog inputs */
  status = VAL_Analog_Init();
  if (status != VAL_OK) {
    return status;
  }
  
  /* Initialize data storage */
  // TODO
//  status = VAL_DataStore_Init();
//  if (status != VAL_OK) {
//    return status;
//  }
  
  return VAL_OK;
}

/**
  * @brief  De-initialize all VAL modules
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
static inline VAL_Status VAL_DeInit(void) {
  VAL_Status status = VAL_OK;
  VAL_Status moduleStatus;
  
  /* De-initialize data storage */
  // TODO
//  moduleStatus = VAL_DataStore_DeInit();
//  if (moduleStatus != VAL_OK) {
//    status = moduleStatus;
//  }
  
  /* De-initialize analog inputs */
  moduleStatus = VAL_Analog_DeInit();
  if (moduleStatus != VAL_OK) {
    status = moduleStatus;
  }
  
  /* De-initialize PWM */
  moduleStatus = VAL_PWM_DeInit();
  if (moduleStatus != VAL_OK) {
    status = moduleStatus;
  }
  
  /* De-initialize GPIO pins */
  moduleStatus = VAL_Pins_DeInit();
  if (moduleStatus != VAL_OK) {
    status = moduleStatus;
  }
  
  return status;
}

#ifdef __cplusplus
}
#endif

#endif /* __VAL_H */
