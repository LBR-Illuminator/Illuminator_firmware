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
  {1, 0.0f, 0.0f},  // First light, initialized with ID and zero values
  {2, 0.0f, 0.0f},  // Second light
  {3, 0.0f, 0.0f}   // Third light
};

static uint8_t light_alarms[3] = {0, 0, 0};
static uint8_t previous_light_alarms[3] = {0, 0, 0};

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
VAL_Status SYS_Coordinator_GetLightIntensity(uint8_t light_id, uint8_t* intensity) {
  /* Validate light ID */
  if (light_id < 1 || light_id > 3) {
    return VAL_ERROR;
  }

  /* Retrieve the current intensity for the specified light */
  *intensity = current_intensities[light_id - 1];
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
VAL_Status SYS_Coordinator_SetLightIntensity(uint8_t light_id, uint8_t intensity) {
  /* Validate light ID */
  if (light_id < 1 || light_id > 3) {
    return VAL_ERROR;
  }

  /* Validate intensity */
  if (intensity > 100) {
    return VAL_ERROR;
  }

  /* Set the intensity for the specified light */
  VAL_Status status = LED_Driver_SetIntensity(light_id, intensity);
  if (status == VAL_OK) {
    current_intensities[light_id - 1] = intensity;
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
VAL_Status SYS_Coordinator_GetLightSensorData(uint8_t light_id, LightSensorData_t* sensor_data) {
  /* Validate input parameters */
  if (light_id < 1 || light_id > 3 || sensor_data == NULL) {
    return VAL_ERROR;
  }

  /* Get sensor data from LED driver */
  VAL_Status status = LED_Driver_GetSensorData(light_id, sensor_data);

  /* If successful, update our cached copy */
  if (status == VAL_OK) {
    current_sensor_data[light_id - 1] = *sensor_data;
  }

  return status;
}

/**
 * @brief Get sensor data for all light sources
 * @param sensorData Array to store sensor readings (must be size 3)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_GetAllLightSensorData(LightSensorData_t* sensor_data) {
  /* Validate input parameter */
  if (sensor_data == NULL) {
    return VAL_ERROR;
  }

  memcpy(sensor_data, current_sensor_data, 3 * sizeof(LightSensorData_t));

  return VAL_OK;
}

/**
 * @brief Clear alarm for a specific light source
 * @param lightId Light source ID (1-3)
 * @return VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status SYS_Coordinator_ClearLightAlarm(uint8_t light_id) {
  /* Validate light ID */
  if (light_id < 1 || light_id > 3) {
    return VAL_ERROR;
  }

  /* Attempt to clear the alarm in the LED driver */
  return LED_Driver_ClearAlarm(light_id);
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
  memcpy(alarms, light_alarms, sizeof(alarms));
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
    VAL_Status status;

    /* Task main loop */
    for (;;) {
        /* Synchronize all light intensities */
        status = LED_Driver_GetAllIntensities(current_intensities);
        if(status != VAL_OK)
          VAL_Serial_Printf("Failed to get intensities\n");

        /* Synchronize all sensor data */
        status = LED_Driver_GetAllSensorData(current_sensor_data);
        if(status != VAL_OK)
          VAL_Serial_Printf("Failed to get sensor data\n");

        /* Synchronize all alarms */
        status = LED_Driver_GetAlarmStatus(light_alarms);
        if(status != VAL_OK)
          VAL_Serial_Printf("Failed to get alarms\n");

        /* Check for new alarm conditions */
        for (uint8_t i = 0; i < 3; i++) {
            if (light_alarms[i] != 0 && previous_light_alarms[i] == 0) {
                /* New alarm detected - send event notification */
                float value = 0.0f;

                /* Use the appropriate sensor value based on alarm type */
                if (light_alarms[i] == 1) { /* ERROR_OVER_CURRENT */
                    value = current_sensor_data[i].current;
                } else if (light_alarms[i] == 2) { /* ERROR_OVER_TEMPERATURE */
                    value = current_sensor_data[i].temperature;
                }

                /* Send alarm event notification */
                COMMS_Handler_SendAlarmEvent(i + 1, light_alarms[i], value);

                /* Log the alarm event */
//                VAL_Serial_Printf("Alarm triggered for light %d: type %d, value %.1f\n",
//                                 i + 1, light_alarms[i], value);
            }

            /* Update previous alarm state */
            previous_light_alarms[i] = light_alarms[i];
        }

        osDelay(100); /* Update every 100 milliseconds */
    }
}
