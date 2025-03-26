/**
  ******************************************************************************
  * @file    app_led_driver.c
  * @brief   LED Driver implementation for Wiseled_LBR
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_led_driver.h"
#include "val.h"

/* Private define ------------------------------------------------------------*/
#define NUM_LIGHT_SOURCES 3

/* Current limits for each light source in mA */
#define LIGHT_CURRENT_MAX   50.0f  /* 500 mA maximum current */
#define LIGHT_CURRENT_MIN   0.0f    /* Minimum allowed current */

/* Temperature limits for each light source in Celsius */
#define LIGHT_TEMP_MAX      85.0f   /* Maximum allowed temperature */
#define LIGHT_TEMP_MIN      0.0f    /* Minimum allowed temperature */

/* Private variables ---------------------------------------------------------*/
static uint8_t current_intensities[NUM_LIGHT_SOURCES] = {0, 0, 0};

/* Sensor data storage */
static LightSensorData_t light_sensor_data[NUM_LIGHT_SOURCES] = {
    {1, 0.0f, 0.0f},  // First light, initialized with ID and zero values
    {2, 0.0f, 0.0f},  // Second light
    {3, 0.0f, 0.0f}   // Third light
};

static uint8_t light_alarms[NUM_LIGHT_SOURCES] = {0, 0, 0};

/* Private function prototypes -----------------------------------------------*/
static VAL_Status validate_light_id(uint8_t lightId);
static void check_alarm_conditions(void);
static VAL_Status update_sensor_readings(void);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the LED driver
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_Init(void) {
  /* Initialize analog sensors */
  VAL_Status analog_status = VAL_Analog_Init();
  if (analog_status != VAL_OK) {
    return analog_status;
  }

  /* Set initial values for all lights */
  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    current_intensities[i] = 0;
    light_alarms[i] = 0;
  }

  /* Initialize PWM channels to 0% intensity */
  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    VAL_PWM_SetIntensity(i + 1, 0);
  }

  return VAL_OK;
}

/**
 * @brief  Set the intensity for a specific light source
 * @param  lightId: Light source ID (1-3)
 * @param  intensity: Intensity value (0-100)
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_SetIntensity(uint8_t lightId, uint8_t intensity) {
  VAL_Status status;

  /* Validate input */
  status = validate_light_id(lightId);
  if (status != VAL_OK) {
    return status;
  }

  if (intensity > 100) {
    return VAL_ERROR;
  }

  /* Check if there's an active alarm for this light */
  if (light_alarms[lightId - 1]) {
    /* Cannot set intensity when light has an active alarm */
    return VAL_ERROR;
  }

  /* Update sensor data first */
  status = update_sensor_readings();
  if (status != VAL_OK) {
    return status;
  }

  /* Store current intensity */
  current_intensities[lightId - 1] = intensity;

  /* Check for potential alarm conditions */
  check_alarm_conditions();

  /* Set the actual PWM output for the light source */
  return VAL_PWM_SetIntensity(lightId, intensity);
}

/**
 * @brief  Set intensities for all light sources
 * @param  intensities: Array of intensity values (0-100)
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_SetAllIntensities(uint8_t *intensities) {
  VAL_Status status = VAL_OK;
  VAL_Status pwmStatus = VAL_OK;

  /* Update sensor data first */
  status = update_sensor_readings();
  if (status != VAL_OK) {
    return status;
  }

  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    /* Skip lights with active alarms instead of failing the whole operation */
    if (!light_alarms[i]) {
      /* Update internal state */
      current_intensities[i] = intensities[i];

      /* Set the actual PWM output */
      VAL_Status lightStatus = VAL_PWM_SetIntensity(i + 1, intensities[i]);
      if (lightStatus != VAL_OK) {
        /* Return error but continue setting other lights */
        status = VAL_ERROR;
        pwmStatus = VAL_ERROR;
      }
    }
  }

  /* Check alarm conditions */
  check_alarm_conditions();

  return status;
}

/**
 * @brief  Get the current intensity of a specific light source
 * @param  lightId: Light source ID (1-3)
 * @param  intensity: Pointer to store the retrieved intensity
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_GetIntensity(uint8_t lightId, uint8_t* intensity) {
  /* Validate light ID */
  if (lightId < 1 || lightId > NUM_LIGHT_SOURCES) {
    return VAL_ERROR;
  }

  /* Retrieve the intensity */
  *intensity = current_intensities[lightId - 1];

  return VAL_OK;
}

/**
 * @brief  Get intensities for all light sources
 * @param  intensities: Array to store retrieved intensities
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_GetAllIntensities(uint8_t* intensities) {
  /* Copy current intensities to the provided array */
  memcpy(intensities, current_intensities, NUM_LIGHT_SOURCES * sizeof(uint8_t));
  return VAL_OK;
}

/**
 * @brief  Get current sensor readings
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
static VAL_Status update_sensor_readings(void) {
  VAL_Status status;

  /* Get sensor data from analog inputs */
  status = VAL_Analog_GetAllSensorData((LightSensorData*)light_sensor_data);

  /* Check alarm conditions */
  check_alarm_conditions();

  return status;
}

/**
 * @brief  Check for and handle alarm conditions
 * @retval None
 */
static void check_alarm_conditions(void) {
  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    /* Disable light if current exceeds max or falls below min */
    if (light_sensor_data[i].current > LIGHT_CURRENT_MAX ||
        light_sensor_data[i].current < LIGHT_CURRENT_MIN) {
      /* Set alarm and turn off light */
      light_alarms[i] = ERROR_OVER_CURRENT;
      current_intensities[i] = 0;

      /* Turn off the actual PWM output */
      VAL_PWM_SetIntensity(i + 1, 0);
    }

    /* Disable light if temperature exceeds max or falls below min */
    if (light_sensor_data[i].temperature > LIGHT_TEMP_MAX ||
        light_sensor_data[i].temperature < LIGHT_TEMP_MIN) {
      /* Set alarm and turn off light */
      light_alarms[i] = ERROR_OVER_TEMPERATURE;
      current_intensities[i] = 0;

      /* Turn off the actual PWM output */
      VAL_PWM_SetIntensity(i + 1, 0);
    }
  }
}

/**
 * @brief  Validate light source ID
 * @param  lightId: Light source ID to validate
 * @retval VAL_Status: VAL_OK if valid, VAL_ERROR otherwise
 */
static VAL_Status validate_light_id(uint8_t lightId) {
  if (lightId < 1 || lightId > NUM_LIGHT_SOURCES) {
    return VAL_ERROR;
  }
  return VAL_OK;
}

/**
 * @brief  Get sensor readings for a specific light source
 * @param  lightId: Light source ID (1-3)
 * @param  sensorData: Pointer to store the retrieved sensor data
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_GetSensorData(uint8_t lightId, LightSensorData_t* sensorData) {
  /* Validate input */
  VAL_Status status = validate_light_id(lightId);
  if (status != VAL_OK) {
    return status;
  }

  if (sensorData == NULL) {
    return VAL_ERROR;
  }

  /* Update sensor data first */
  status = update_sensor_readings();
  if (status != VAL_OK) {
    return status;
  }

  /* Copy the sensor data for the specified light */
  *sensorData = light_sensor_data[lightId - 1];

  return VAL_OK;
}

/**
 * @brief  Get sensor readings for all light sources
 * @param  sensorData: Array to store the retrieved sensor data (must be size 3)
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_GetAllSensorData(LightSensorData_t* sensorData) {
  /* Validate input */
  if (sensorData == NULL) {
    return VAL_ERROR;
  }

  /* Update sensor data first */
  VAL_Status status = update_sensor_readings();
  if (status != VAL_OK) {
    return status;
  }

  /* Copy all sensor data to the provided array */
  memcpy(sensorData, light_sensor_data, NUM_LIGHT_SOURCES * sizeof(LightSensorData_t));

  return VAL_OK;
}

/**
 * @brief  Clear alarm for a specific light source
 * @param  lightId: Light source ID (1-3)
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_ClearAlarm(uint8_t lightId) {
  /* Validate input */
  VAL_Status status = validate_light_id(lightId);
  if (status != VAL_OK) {
    return status;
  }

  /* Update sensor data to ensure we're checking current conditions */
  status = update_sensor_readings();
  if (status != VAL_OK) {
    return status;
  }

  /* Check if conditions are still in alarm state */
  if (light_sensor_data[lightId - 1].current > LIGHT_CURRENT_MAX ||
      light_sensor_data[lightId - 1].temperature > LIGHT_TEMP_MAX ||
      light_sensor_data[lightId - 1].current < LIGHT_CURRENT_MIN ||
      light_sensor_data[lightId - 1].temperature < LIGHT_TEMP_MIN) {
    /* Cannot clear alarm - conditions are still present */
    return VAL_ERROR;
  }

  /* Clear the alarm */
  light_alarms[lightId - 1] = 0;

  return VAL_OK;
}

/**
 * @brief  Get alarm status for all light sources
 * @param  alarms: Array to store alarm status (must be size 3)
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_GetAlarmStatus(uint8_t* alarms) {
  /* Validate input */
  if (alarms == NULL) {
    return VAL_ERROR;
  }

  /* Copy alarm status to the provided array */
  memcpy(alarms, light_alarms, NUM_LIGHT_SOURCES * sizeof(uint8_t));
  return VAL_OK;
}
