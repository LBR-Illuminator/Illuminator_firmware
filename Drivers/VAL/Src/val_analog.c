/**
  ******************************************************************************
  * @file    val_analog.c
  * @brief   Vendor Abstraction Layer for analog sensing
  ******************************************************************************
  * @attention
  *
  * This module provides a hardware-independent interface for analog
  * input reading used by the Wiseled_LBR system for current and 
  * temperature monitoring.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "val_analog.h"
#include "val_serial_comms.h"
#include "adc.h"
#include "dma.h"
#include <string.h>
#include <math.h>

/* Private define ------------------------------------------------------------*/
#define ADC_CHANNEL_COUNT 6
#define LIGHT_COUNT 3
#define ADC_BUFFER_SIZE ADC_CHANNEL_COUNT
#define ADC_RESOLUTION 4095.0f  /* 12-bit ADC */
#define ADC_REFERENCE 3.3f      /* Reference voltage in volts */

/* Simplified sensor conversion factors */
#define CURRENT_CONVERSION_FACTOR 10.0f       /* 3.3V = 33A, so each volt is 10A */
#define TEMPERATURE_CONVERSION_FACTOR 100.0f  /* 3.3V = 330°C, so each volt is 100°C */

/* Private typedef -----------------------------------------------------------*/
typedef enum {
  CHANNEL_CURRENT_1 = 0,
  CHANNEL_CURRENT_2 = 1,
  CHANNEL_CURRENT_3 = 2,
  CHANNEL_TEMP_1 = 3,
  CHANNEL_TEMP_2 = 4,
  CHANNEL_TEMP_3 = 5
} AdcChannelIndex;

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t adc_buffer[ADC_BUFFER_SIZE];
static volatile uint8_t conversion_complete = 0;

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the analog module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_Init(void) {
  VAL_Serial_Printf("Starting ADC initialization...\r\n");

  /* Stop any ongoing conversions first */
  HAL_ADC_Stop_DMA(&hadc1);

  /* Initialize DMA first - this is critical */
  MX_DMA_Init();

  /* Reset ADC */
  __HAL_ADC_RESET_HANDLE_STATE(&hadc1);

  /* Initialize ADC */
  MX_ADC1_Init();
  
  /* Small delay to let ADC stabilize */
  HAL_Delay(10);

  /* Clear any pending flags */
  __HAL_ADC_CLEAR_FLAG(&hadc1, (ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR));
  
  /* Start ADC in DMA mode */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUFFER_SIZE) != HAL_OK) {
    VAL_Serial_Printf("Failed to start ADC DMA\r\n");
    return VAL_ERROR;
  }
  
  HAL_ADC_Start(&hadc1);

  VAL_Serial_Printf("ADC initialization completed successfully\r\n");
  return VAL_OK;
}

/**
  * @brief  Get current reading for a specific light
  * @param  lightId: Light ID (1-3)
  * @param  current: Pointer to store current value in Amps
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_GetCurrent(uint8_t lightId, float* current) {
  /* Check parameters */
  if (lightId < 1 || lightId > LIGHT_COUNT || current == NULL) {
    return VAL_PARAM;
  }
  
  /* Map light ID to ADC channel */
  AdcChannelIndex channelIndex = CHANNEL_CURRENT_1 + (lightId - 1);
  
  /* Get raw ADC value */
  uint32_t adcValue = adc_buffer[channelIndex];
  
  /* Convert to voltage */
  float voltage = (adcValue / ADC_RESOLUTION) * ADC_REFERENCE;
  
  /* Simple linear conversion: 3.3V = 33A */
  *current = voltage * CURRENT_CONVERSION_FACTOR;
  
  return VAL_OK;
}

/**
  * @brief  Get temperature reading for a specific light
  * @param  lightId: Light ID (1-3)
  * @param  temperature: Pointer to store temperature value in degrees Celsius
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_GetTemperature(uint8_t lightId, float* temperature) {
  /* Check parameters */
  if (lightId < 1 || lightId > LIGHT_COUNT || temperature == NULL) {
    return VAL_PARAM;
  }
  
  /* Map light ID to ADC channel */
  AdcChannelIndex channelIndex = CHANNEL_TEMP_1 + (lightId - 1);
  
  /* Get raw ADC value */
  uint32_t adcValue = adc_buffer[channelIndex];
  
  /* Convert to voltage */
  float voltage = (adcValue / ADC_RESOLUTION) * ADC_REFERENCE;
  
  /* Simple linear conversion: 3.3V = 330°C */
  *temperature = voltage * TEMPERATURE_CONVERSION_FACTOR;
  
  return VAL_OK;
}

/**
  * @brief  Get all sensor readings for a specific light
  * @param  lightId: Light ID (1-3)
  * @param  sensorData: Pointer to store sensor data
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_GetSensorData(uint8_t lightId, LightSensorData* sensorData) {
  /* Check parameters */
  if (lightId < 1 || lightId > LIGHT_COUNT || sensorData == NULL) {
    return VAL_PARAM;
  }
  
  /* Get current and temperature readings */
  VAL_Status status = VAL_Analog_GetCurrent(lightId, &sensorData->current);
  if (status != VAL_OK) {
    return status;
  }
  
  status = VAL_Analog_GetTemperature(lightId, &sensorData->temperature);
  if (status != VAL_OK) {
    return status;
  }

  return VAL_OK;
}

/**
  * @brief  Get sensor readings for all lights
  * @param  sensorData: Array to store sensor data (must be at least LIGHT_COUNT elements)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_GetAllSensorData(LightSensorData sensorData[]) {
  VAL_Status overallStatus = VAL_OK;

  /* Get sensor data for each light */
  for (uint8_t i = 0; i < LIGHT_COUNT; i++) {
    /* Explicitly initialize the sensor data entry */
    sensorData[i].current = 0.0f;
    sensorData[i].temperature = 0.0f;

    /* Get sensor data for this specific light */
    VAL_Status lightStatus = VAL_Analog_GetSensorData(i + 1, &sensorData[i]);

    if (lightStatus != VAL_OK) {
      /* Track if any light's sensor data retrieval failed */
      overallStatus = VAL_ERROR;
    }
  }
  
  return overallStatus;
}

/**
  * @brief  De-initialize the analog module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_DeInit(void) {
  /* Stop ADC conversion */
  HAL_ADC_Stop_DMA(&hadc1);
  
  return VAL_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  ADC conversion complete callback
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	//TODO: implement safety limits check
}

/**
  * @brief  ADC error callback
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc) {
  // Get error code
  uint32_t errorCode = HAL_ADC_GetError(hadc);

  // Print specific error information
  VAL_Serial_Printf("ADC Error occurred, code: 0x%08lX\r\n", errorCode);

  if (errorCode & HAL_ADC_ERROR_INTERNAL) VAL_Serial_Printf("- Internal error\r\n");
  if (errorCode & HAL_ADC_ERROR_OVR) VAL_Serial_Printf("- Overrun error\r\n");
  if (errorCode & HAL_ADC_ERROR_DMA) VAL_Serial_Printf("- DMA transfer error\r\n");
  if (errorCode & HAL_ADC_ERROR_JQOVF) VAL_Serial_Printf("- Injected queue overflow error\r\n");

  // More thorough error recovery
  HAL_ADC_Stop_DMA(hadc);

  // Clear all flags
  __HAL_ADC_CLEAR_FLAG(hadc, (ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR));
  if (hadc->DMA_Handle != NULL) {
    __HAL_DMA_CLEAR_FLAG(hadc->DMA_Handle, (DMA_FLAG_TC1 | DMA_FLAG_HT1 | DMA_FLAG_TE1));
  }

  // Wait a moment
  HAL_Delay(10);

  // Restart ADC with DMA
  HAL_ADC_Start_DMA(hadc, (uint32_t*)adc_buffer, ADC_BUFFER_SIZE);
}

/**
  * @brief  DMA error callback
  * @param  hdma: DMA handle
  * @retval None
  */
void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma) {
  VAL_Serial_Printf("DMA Error occurred, code: 0x%08lX\r\n", HAL_DMA_GetError(hdma));
}
