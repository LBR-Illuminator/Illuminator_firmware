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

/* Private variables ---------------------------------------------------------*/
static uint8_t current_intensities[NUM_LIGHT_SOURCES] = {0, 0, 0};

/* Private function prototypes -----------------------------------------------*/
static VAL_Status validate_light_id(uint8_t lightId);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the LED driver
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_Init(void) {
  /* TODO: Perform any necessary LED driver initialization */
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

  /* Store current intensity */
  current_intensities[lightId - 1] = intensity;

  /* TODO: Implement actual PWM control for the light source */
  // VAL_PWM_SetDutyCycle(lightId, intensity);

  return VAL_OK;
}

/**
 * @brief  Set intensities for all light sources
 * @param  intensities: Array of intensity values (0-100)
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_SetAllIntensities(uint8_t *intensities) {
  for (uint8_t i = 0; i < NUM_LIGHT_SOURCES; i++) {
    VAL_Status status = LED_Driver_SetIntensity(i + 1, intensities[i]);
    if (status != VAL_OK) {
      return status;
    }
  }
  return VAL_OK;
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
 * @brief  Clear alarm for a specific light source
 * @param  lightId: Light source ID (1-3)
 * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status LED_Driver_ClearAlarm(uint8_t lightId) {
  /* TODO: Implement alarm clearing logic */
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
