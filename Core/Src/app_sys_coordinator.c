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

/* Private sensor data storage */
static LightSensorData_t current_sensor_data[3] = {
  {0.0f, 25.0f},  /* Light 1: 0 mA, 25°C */
  {0.0f, 25.0f},  /* Light 2: 0 mA, 25°C */
  {0.0f, 25.0f}   /* Light 3: 0 mA, 25°C */
};


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

/**
 * @brief Set the intensity for a specific light source
 * @param lightId Light source ID (1-3)
 * @param intensity Intensity value (0-100)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_SetLightIntensity(uint8_t lightId, uint8_t intensity) {
  /* Validate light ID */
  if (lightId < 1 || lightId > 3) {
    return VAL_ERROR;
  }

  /* Validate intensity */
  if (intensity > 100) {
    return VAL_ERROR;
  }

  /* Set the intensity for the specified light */
  VAL_Status status = LED_Driver_SetIntensity(lightId, intensity);
  if (status == VAL_OK) {
    current_intensities[lightId - 1] = intensity;
  }

  return status;
}

/**
 * @brief Set intensities for all light sources
 * @param intensities Array of intensity values (0-100)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_SetAllLightIntensities(uint8_t* intensities) {
  /* Validate input */
  if (intensities == NULL) {
    return VAL_ERROR;
  }

  /* Set intensities for all light sources */
  VAL_Status status = LED_Driver_SetAllIntensities(intensities);
  if (status == VAL_OK) {
    memcpy(current_intensities, intensities, 3 * sizeof(uint8_t));
  }

  return status;
}

/**
 * @brief Get sensor data for a specific light source
 * @param lightId Light source ID (1-3)
 * @param sensorData Pointer to store sensor readings
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetLightSensorData(uint8_t lightId, LightSensorData_t* sensorData) {
  /* Validate input parameters */
  if (lightId < 1 || lightId > 3 || sensorData == NULL) {
    return VAL_ERROR;
  }

  /* Get sensor data from LED driver */
  VAL_Status status = LED_Driver_GetSensorData(lightId, sensorData);

  /* If successful, update our cached copy */
  if (status == VAL_OK) {
    current_sensor_data[lightId - 1] = *sensorData;
  }

  return status;
}

/**
 * @brief Get sensor data for all light sources
 * @param sensorData Array to store sensor readings (must be size 3)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetAllLightSensorData(LightSensorData_t* sensorData) {
  /* Validate input parameter */
  if (sensorData == NULL) {
    return VAL_ERROR;
  }

  /* Get all sensor data from LED driver */
  VAL_Status status = LED_Driver_GetAllSensorData(sensorData);

  /* If successful, update our cached copy */
  if (status == VAL_OK) {
    memcpy(current_sensor_data, sensorData, 3 * sizeof(LightSensorData_t));
  }

  return status;
}

/**
 * @brief Clear alarm for a specific light source
 * @param lightId Light source ID (1-3)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_ClearLightAlarm(uint8_t lightId) {
  /* Validate light ID */
  if (lightId < 1 || lightId > 3) {
    return VAL_ERROR;
  }

  /* Attempt to clear the alarm in the LED driver */
  return LED_Driver_ClearAlarm(lightId);
}

/**
 * @brief Get alarm status for all light sources
 * @param alarms Array to store alarm status (must be size 3)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetAlarmStatus(uint8_t* alarms) {
  /* Validate input parameter */
  if (alarms == NULL) {
    return VAL_ERROR;
  }

  /* Get alarm status from LED driver */
  return LED_Driver_GetAlarmStatus(alarms);
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
        /* Synchronize all light intensities */
        LED_Driver_GetAllIntensities(current_intensities);

        /* Synchronize all sensor data */
        LED_Driver_GetAllSensorData(current_sensor_data);

        osDelay(100); /* Update every 100 milliseconds */
    }
}
