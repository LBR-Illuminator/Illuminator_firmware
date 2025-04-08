/**
  ******************************************************************************
  * @file    val_timers.c
  * @brief   Vendor Abstraction Layer for timer functionality
  ******************************************************************************
  * @attention
  *
  * This module provides a hardware-independent interface for timers
  * used by the Wiseled_LBR system.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "val_timers.h"
#include "tim.h"

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize timers
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Timers_Init(void) {
  /* Initialize timer peripherals if needed */
  /* Currently leveraging the HAL initialization */

  return VAL_OK;
}

/**
  * @brief  De-initialize timers
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Timers_DeInit(void) {
  /* De-initialize timer peripherals if needed */

  return VAL_OK;
}

/* Timer Callbacks ----------------------------------------------------------*/

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}
