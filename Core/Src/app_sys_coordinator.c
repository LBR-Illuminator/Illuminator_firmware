/**
  ******************************************************************************
  * @file    app_sys_coordinator.c
  * @brief   Application layer system coordinator module
  ******************************************************************************
  * @attention
  *
  * This module implements a minimal system coordinator.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_sys_coordinator.h"
#include "val.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

/* Private define ------------------------------------------------------------*/
#define SYS_COORDINATOR_STACK_SIZE    256
#define SYS_COORDINATOR_PRIORITY      osPriorityNormal

/* Private variables ---------------------------------------------------------*/
static TaskHandle_t sysCoordinatorTaskHandle = NULL;

/* Private function prototypes -----------------------------------------------*/
static void SYS_Coordinator_Task(void const *argument);

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the system coordinator
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status SYS_Coordinator_Init(void) {
  /* Create system coordinator task */
  osThreadDef(SysCoordTask, SYS_Coordinator_Task, SYS_COORDINATOR_PRIORITY, 0, SYS_COORDINATOR_STACK_SIZE);
  sysCoordinatorTaskHandle = osThreadCreate(osThread(SysCoordTask), NULL);

  if (sysCoordinatorTaskHandle == NULL) {
    return VAL_ERROR;
  }

  return VAL_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  System coordinator task
  * @param  argument: Task argument
  * @retval None
  */
static void SYS_Coordinator_Task(void const *argument) {
  /* Initialize */
  VAL_Serial_Printf("System Coordinator initialized\r\n");

  /* Task main loop */
  for (;;) {
    /* Currently just a placeholder for future functionality */
    osDelay(1000);
  }
}
