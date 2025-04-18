/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "val.h"
#include "app_comms_handler.h"
#include "app_sys_coordinator.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static TaskHandle_t init_task_handle = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
static void System_Init(void);
static void System_Error(void);
static void Init_Task(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief Initialize system components
  * @retval None
  */
static void System_Init(void) {
  VAL_Status status;

  /* Initialize all Vendor Abstraction Layer modules */
  status = VAL_Init();
  if (status != VAL_OK) {
    System_Error();
  }

  /* Create initialization task to complete initialization after FreeRTOS starts */
  osThreadDef(InitTask, Init_Task, osPriorityHigh, 0, 256);
  init_task_handle = osThreadCreate(osThread(InitTask), NULL);

  if (init_task_handle == NULL) {
    System_Error();
  }
}

/**
  * @brief Application initialization task
  * @param  argument: Not used
  * @retval None
  */
static void Init_Task(void *argument) {
  VAL_Status status;

  /* Initialize communications handler */
  status = COMMS_Handler_Handler_Init();
  if (status != VAL_OK) {
    System_Error();
  }

  /* Initialize System Coordinator*/
  status = SYS_Coordinator_Init();
  if (status != VAL_OK) {
	  System_Error();
  }

  /* Send system ready message */
  VAL_Serial_Printf("Wiseled_LBR System ready!\r\n");

  /* Delete the init task as it's no longer needed */
  vTaskDelete(NULL);

}

/**
  * @brief Handle system initialization error
  * @retval None
  */
static void System_Error(void) {
  /* Initialization error occurred */
  while (1) {
    /* Toggle LED rapidly to indicate error */
	VAL_Pins_ToggleBoardLED();
	VAL_SysClock_Delay(100);
  }
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Initialize all configured peripherals through VAL */
  System_Init();

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
