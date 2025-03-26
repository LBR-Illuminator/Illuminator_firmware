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
#define LIGHT_CURRENT_MAX   500.0f
#define LIGHT_CURRENT_MIN   0.0f

/* Temperature limits for each light source in Celsius */
#define LIGHT_TEMP_MAX      85.0f
#define LIGHT_TEMP_MIN      0.0f

/* Define error codes */
#define ERROR_OVER_CURRENT     0
#define ERROR_OVER_TEMPERATURE 1
#define ERROR_SYSTEM           2

/* Private variables ---------------------------------------------------------*/
static uint8_t current_intensities[NUM_LIGHT_SOURCES] = {0, 0, 0};

/* Mock sensor data for simulation purposes */
static LightSensorData_t light_sensor_data[NUM_LIGHT_SOURCES] = {
  {0.0f, 0.0f},
  {0.0f, 0.0f},
  {0.0f, 0.0f}
};

/* Alarm status for each light */
static uint8_t light_alarms[NUM_LIGHT_SOURCES] = {0, 0, 0};

/* Private function prototypes -----------------------------------------------*/
static VAL_Status validate_light_id(uint8_t lightId);
static void update_sensor_data(void);
static void check_alarm_conditions(void);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the LED driver
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_Init(void) {
  /* Set initial values for all lights */
  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    current_intensities[i] = 0;
    light_sensor_data[i].current = 0.0f;
    light_sensor_data[i].temperature = 25.0f;
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

  /* Store current intensity */
  current_intensities[lightId - 1] = intensity;

  /* Update mock sensor data based on new intensity */
  update_sensor_data();

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

  /* Update sensor data and check alarms after setting all intensities */
  update_sensor_data();
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

  /* Update mock sensor data before returning values */
  update_sensor_data();

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

  /* Update mock sensor data before returning values */
  update_sensor_data();

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
  update_sensor_data();

  /* Check if conditions are still in alarm state */
  if (light_sensor_data[lightId - 1].current > LIGHT_CURRENT_MAX ||
      light_sensor_data[lightId - 1].temperature > LIGHT_TEMP_MAX) {
    /* Cannot clear alarm - conditions are still present */
    return VAL_ERROR;
  }

  /* Clear the alarm */
  light_alarms[lightId - 1] = 0;

  return VAL_OK;
}

/* Private functions ---------------------------------------------------------*/

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
 * @brief  Update mock sensor data based on current intensities
 * @retval None
 */
static void update_sensor_data(void) {
  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    /* Update current based on intensity (linear relationship for mock data) */
    light_sensor_data[i].current = (current_intensities[i] / 100.0f) * 350.0f; /* Max 350mA */

    /* Update temperature based on intensity with some randomness */
    float temp_increase = (current_intensities[i] / 100.0f) * 40.0f; /* Up to 40°C increase from base */
    light_sensor_data[i].temperature = 25.0f + temp_increase;

    /* Add small random variations for more realistic test data */
    light_sensor_data[i].current += (float)((rand() % 10) - 5);     /* ±5mA variation */
    light_sensor_data[i].temperature += (float)((rand() % 4) - 2) / 10.0f; /* ±0.2°C variation */

    /* Ensure values stay within valid ranges */
    if (light_sensor_data[i].current < 0.0f) light_sensor_data[i].current = 0.0f;
    if (light_sensor_data[i].temperature < 25.0f) light_sensor_data[i].temperature = 25.0f;
  }
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

/**
 * @brief  Check for and handle alarm conditions
 * @retval None
 */
static void check_alarm_conditions(void) {
  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    /* Check for over-current condition */
    if (light_sensor_data[i].current > LIGHT_CURRENT_MAX) {
      /* Set alarm and turn off light */
      light_alarms[i] = ERROR_OVER_CURRENT;
      current_intensities[i] = 0;

      /* Turn off the actual PWM output */
      VAL_PWM_SetIntensity(i + 1, 0);

      /* TODO: Log error event */
    }

    /* Check for over-temperature condition */
    if (light_sensor_data[i].temperature > LIGHT_TEMP_MAX) {
      /* Set alarm and turn off light */
      light_alarms[i] = ERROR_OVER_TEMPERATURE;
      current_intensities[i] = 0;

      /* Turn off the actual PWM output */
      VAL_PWM_SetIntensity(i + 1, 0);

      /* TODO: Log error event */
    }
  }
}
