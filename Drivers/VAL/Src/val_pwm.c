/**
  ******************************************************************************
  * @file    val_pwm.c
  * @brief   Vendor Abstraction Layer for PWM control
  ******************************************************************************
  * @attention
  *
  * This module provides a hardware-independent interface for PWM
  * control used by the Wiseled_LBR system for light intensity control.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "val_pwm.h"
#include "tim.h"

/* Private define ------------------------------------------------------------*/
#define PWM_CHANNEL_COUNT 3
#define PWM_MAX_INTENSITY 100

/* Private variables ---------------------------------------------------------*/
static const uint32_t pwmChannels[PWM_CHANNEL_COUNT] = {
  TIM_CHANNEL_1,  /* LED1 - White */
  TIM_CHANNEL_2,  /* LED2 - Green */
  TIM_CHANNEL_3   /* LED3 - Red */
};

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the PWM module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_PWM_Init(void) {
  /* Initialize PWM timer */
  MX_TIM1_Init();
  
  /* Start PWM generation for all channels */
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
    return VAL_ERROR;
  }
  
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    return VAL_ERROR;
  }
  
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    return VAL_ERROR;
  }
  
  /* Initialize all channels to 0% intensity */
  for (uint8_t i = 0; i < PWM_CHANNEL_COUNT; i++) {
    VAL_PWM_SetIntensity(i + 1, 0);
  }
  
  return VAL_OK;
}

/**
  * @brief  Set PWM intensity for a specific channel
  * @param  channel: Channel number (1-3)
  * @param  intensity: Intensity value (0-100)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_PWM_SetIntensity(uint8_t channel, uint8_t intensity) {
  /* Check parameters */
  if (channel < 1 || channel > PWM_CHANNEL_COUNT) {
    return VAL_PARAM;
  }
  
  if (intensity > PWM_MAX_INTENSITY) {
    intensity = PWM_MAX_INTENSITY;
  }
  
  /* Set PWM duty cycle */
  __HAL_TIM_SET_COMPARE(&htim1, pwmChannels[channel - 1], intensity);
  
  return VAL_OK;
}

/**
  * @brief  Get current PWM intensity for a specific channel
  * @param  channel: Channel number (1-3)
  * @param  intensity: Pointer to store intensity value (0-100)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_PWM_GetIntensity(uint8_t channel, uint8_t* intensity) {
  /* Check parameters */
  if (channel < 1 || channel > PWM_CHANNEL_COUNT || intensity == NULL) {
    return VAL_PARAM;
  }
  
  /* Get current PWM duty cycle */
  uint32_t compare = __HAL_TIM_GET_COMPARE(&htim1, pwmChannels[channel - 1]);
  
  /* Convert compare value to intensity */
  *intensity = (uint8_t)compare;
  
  return VAL_OK;
}

/**
  * @brief  Set PWM intensity for all channels
  * @param  intensities: Array of intensity values (0-100) for each channel
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_PWM_SetAllIntensities(const uint8_t intensities[]) {
  VAL_Status status = VAL_OK;
  
  /* Set intensity for each channel */
  for (uint8_t i = 0; i < PWM_CHANNEL_COUNT; i++) {
    VAL_Status channelStatus = VAL_PWM_SetIntensity(i + 1, intensities[i]);
    if (channelStatus != VAL_OK) {
      status = channelStatus;
    }
  }
  
  return status;
}

/**
  * @brief  Stop PWM output for a specific channel
  * @param  channel: Channel number (1-3)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_PWM_StopChannel(uint8_t channel) {
  /* Check parameters */
  if (channel < 1 || channel > PWM_CHANNEL_COUNT) {
    return VAL_PARAM;
  }
  
  /* Set intensity to 0 and stop PWM generation */
  __HAL_TIM_SET_COMPARE(&htim1, pwmChannels[channel - 1], 0);
  
  return VAL_OK;
}

/**
  * @brief  De-initialize the PWM module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_PWM_DeInit(void) {
  /* Stop PWM generation for all channels */
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
  
  return VAL_OK;
}
