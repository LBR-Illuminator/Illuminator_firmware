/**
  ******************************************************************************
  * @file    rcc.h
  * @brief   HAL layer for RCC configuration
  ******************************************************************************
  * @attention
  *
  * This module provides functions for configuring the STM32 RCC (Reset and Clock
  * Control) subsystem.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RCC_H
#define __RCC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* __RCC_H */
