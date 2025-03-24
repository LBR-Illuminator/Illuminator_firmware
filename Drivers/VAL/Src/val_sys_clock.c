/**
  ******************************************************************************
  * @file    val_sys_clock.c
  * @brief   Vendor Abstraction Layer for system clock
  ******************************************************************************
  * @attention
  *
  * This module provides a hardware-independent interface for system clock
  * configuration used by the Wiseled_LBR system.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "val_sys_clock.h"
#include "rcc.h"

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the system clock
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_SysClock_Init(void) {
  /* Configure the system clock */
  SystemClock_Config();
  
  return VAL_OK;
}

/**
  * @brief  Get the system tick count
  * @retval uint32_t: Current tick count in milliseconds
  */
uint32_t VAL_SysClock_GetTick(void) {
  return HAL_GetTick();
}

/**
  * @brief  Delay execution for a specified number of milliseconds
  * @param  delay: Delay in milliseconds
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_SysClock_Delay(uint32_t delay) {
  HAL_Delay(delay);
  return VAL_OK;
}

/**
  * @brief  Get the current system clock frequency
  * @retval uint32_t: Clock frequency in Hz
  */
uint32_t VAL_SysClock_GetFrequency(void) {
  return SystemCoreClock;
}

/**
  * @brief  Get current time in microseconds
  * @retval uint32_t: Current time in microseconds
  */
uint32_t VAL_SysClock_GetMicros(void) {
  /* Get the current tick count */
  uint32_t tick = HAL_GetTick();
  
  /* Get the SysTick counter value */
  uint32_t count = SysTick->VAL;
  
  /* Calculate microseconds from tick and counter */
  uint32_t subtick = ((SysTick->LOAD - count) * 1000) / SysTick->LOAD;
  
  return (tick * 1000) + subtick;
}

/**
  * @brief  System Clock Configuration
  * @note   This function is imported from main.c
  * @retval None
  */
extern void SystemClock_Config(void);
