/**
  ******************************************************************************
  * @file    val_serial_comms.c
  * @brief   Vendor Abstraction Layer for Serial Communications
  ******************************************************************************
  * @attention
  *
  * This module provides a hardware-independent interface for serial
  * communication used by the Wiseled_LBR system.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "val_serial_comms.h"
#include "usart.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* Private define ------------------------------------------------------------*/
#define SERIAL_TX_BUFFER_SIZE 256

/* Private variables ---------------------------------------------------------*/
static uint8_t txBuffer[SERIAL_TX_BUFFER_SIZE];
static volatile uint8_t txBusy = 0;
static SerialRxCallback rxCallback = NULL;
static uint8_t rxBuffer[1];  // Single byte buffer for continuous reception

/* Private function prototypes -----------------------------------------------*/
static void StartReceive(void);

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the serial communication module
  * @param  callback: Optional callback function for received data
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Serial_Init(SerialRxCallback callback) {
  /* Initialize UART peripheral */
  MX_USART1_UART_Init();

  /* Store callback function */
  rxCallback = callback;

  /* Start continuous reception if callback is provided */
  if (rxCallback != NULL) {
    StartReceive();
  }

  return VAL_OK;
}

/**
  * @brief  Send data over serial interface
  * @param  data: Pointer to data buffer
  * @param  length: Length of data to send
  * @param  timeout: Timeout in milliseconds
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Serial_Send(const uint8_t* data, uint16_t length, uint32_t timeout) {
  HAL_StatusTypeDef status;

  /* Check if previous transmission is ongoing */
  if (txBusy) {
    return VAL_BUSY;
  }

  txBusy = 1;

  /* Send data via UART */
  status = HAL_UART_Transmit(&huart1, (uint8_t*)data, length, timeout);

  txBusy = 0;

  return (status == HAL_OK) ? VAL_OK : VAL_ERROR;
}

/**
  * @brief  Send formatted string over serial interface
  * @param  format: String format (printf style)
  * @param  ...: Variable arguments
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Serial_Printf(const char* format, ...) {
  va_list args;
  int length;

  /* Check if previous transmission is ongoing */
  if (txBusy) {
    return VAL_BUSY;
  }

  /* Format string */
  va_start(args, format);
  length = vsnprintf((char*)txBuffer, SERIAL_TX_BUFFER_SIZE, format, args);
  va_end(args);

  if (length < 0 || length >= SERIAL_TX_BUFFER_SIZE) {
    return VAL_ERROR;
  }

  /* Send formatted string */
  return VAL_Serial_Send(txBuffer, length, 1000);
}

/**
  * @brief  Check if serial module is busy transmitting
  * @retval uint8_t: 1 if busy, 0 if ready
  */
uint8_t VAL_Serial_IsBusy(void) {
  return txBusy;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Start continuous reception
  * @retval None
  */
static void StartReceive(void) {
  /* Start UART reception in interrupt mode for a single byte */
  HAL_UART_Receive_IT(&huart1, rxBuffer, 1);
}

/**
  * @brief  UART RX complete callback
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    /* Call user callback with received byte */
    if (rxCallback != NULL) {
      rxCallback(rxBuffer[0]);
    }

    /* Restart reception for next byte */
    HAL_UART_Receive_IT(&huart1, rxBuffer, 1);
  }
}

/**
  * @brief  UART error callback
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    /* Restart reception on error */
    HAL_UART_Receive_IT(&huart1, rxBuffer, 1);
  }
}
