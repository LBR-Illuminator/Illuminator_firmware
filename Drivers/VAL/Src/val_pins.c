/**
  ******************************************************************************
  * @file    val_pins.c
  * @brief   Vendor Abstraction Layer for GPIO pins
  ******************************************************************************
  * @attention
  *
  * This module provides a hardware-independent interface for GPIO pins
  * used by the Wiseled_LBR system.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "val_pins.h"
#include "gpio.h"

/* Private define ------------------------------------------------------------*/
#define BOARD_LED_PIN LD3_Pin
#define BOARD_LED_PORT LD3_GPIO_Port

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the pins module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Pins_Init(void) {
  /* Initialize GPIO */
  MX_GPIO_Init();
  
  return VAL_OK;
}

/**
  * @brief  Set the state of the onboard LED
  * @param  state: LED state (0 = off, 1 = on)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Pins_SetBoardLED(uint8_t state) {
  /* Set LED state */
  HAL_GPIO_WritePin(BOARD_LED_PORT, BOARD_LED_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
  
  return VAL_OK;
}

/**
  * @brief  Toggle the state of the onboard LED
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Pins_ToggleBoardLED(void) {
  /* Toggle LED state */
  HAL_GPIO_TogglePin(BOARD_LED_PORT, BOARD_LED_PIN);
  
  return VAL_OK;
}

/**
  * @brief  Get the state of the onboard LED
  * @param  state: Pointer to store LED state (0 = off, 1 = on)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Pins_GetBoardLED(uint8_t* state) {
  /* Check parameters */
  if (state == NULL) {
    return VAL_PARAM;
  }
  
  /* Get LED state */
  *state = HAL_GPIO_ReadPin(BOARD_LED_PORT, BOARD_LED_PIN) == GPIO_PIN_SET ? 1 : 0;
  
  return VAL_OK;
}

/**
  * @brief  De-initialize the pins module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Pins_DeInit(void) {
  /* Nothing to do for GPIO de-initialization */
  return VAL_OK;
}
