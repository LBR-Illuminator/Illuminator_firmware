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
#include "app_led_driver.h"  // For LED intensity functions
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

/* Private light intensity state storage */
static uint8_t current_intensities[3] = {0, 0, 0};

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

/**
 * @brief Get the current intensity of a specific light source
 * @param lightId Light source ID (1-3)
 * @param intensity Pointer to store the retrieved intensity
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetLightIntensity(uint8_t lightId, uint8_t* intensity) {
  /* Validate light ID */
  if (lightId < 1 || lightId > 3) {
    return VAL_ERROR;
  }

  /* Retrieve the current intensity for the specified light */
  *intensity = current_intensities[lightId - 1];
  return VAL_OK;
}

/**
 * @brief Get the current intensities of all light sources
 * @param intensities Array to store the retrieved intensities
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetAllLightIntensities(uint8_t* intensities) {
  /* Copy the current intensities to the provided array */
  memcpy(intensities, current_intensities, 3 * sizeof(uint8_t));
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
        /* Synchronize all light intensities at once */
        LED_Driver_GetAllIntensities(current_intensities);

        osDelay(100); /* Update every 100 milliseconds */
    }
}
