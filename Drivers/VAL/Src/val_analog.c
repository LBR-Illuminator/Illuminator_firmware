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
#include <string.h>

/* Private define ------------------------------------------------------------*/
#define ADC_CHANNEL_COUNT 6
#define LIGHT_COUNT 3
#define ADC_BUFFER_SIZE ADC_CHANNEL_COUNT
#define ADC_RESOLUTION 4095.0f  /* 12-bit ADC */
#define ADC_REFERENCE 3.3f      /* Reference voltage in volts */

/* Current sensor parameters */
#define CURRENT_SENSE_RATIO 0.1f  /* V/A - Volts per Amp */

/* Temperature sensor parameters */
#define TEMP_SENSOR_BETA 3950.0f
#define TEMP_SENSOR_R0 10000.0f  /* 10k NTC thermistor */
#define TEMP_SENSOR_T0 298.15f   /* 25°C in Kelvin */

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
static volatile uint32_t adcBuffer[ADC_BUFFER_SIZE];
static volatile uint8_t conversionComplete = 0;

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the analog module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_Init(void) {
  /* Stop any ongoing conversions first */
  if (HAL_ADC_Stop(&hadc1) != HAL_OK) {
    VAL_Serial_Printf("Failed to stop ADC\r\n");
  }

  /* Reset ADC */
  __HAL_ADC_RESET_HANDLE_STATE(&hadc1);

  /* Initialize ADC */
  MX_ADC1_Init();
  
  /* Calibrate ADC */
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
    VAL_Serial_Printf("ADC Calibration Failed\r\n");
    return VAL_ERROR;
  }
  
  /* Start ADC in DMA mode */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcBuffer, ADC_CHANNEL_COUNT) != HAL_OK) {
    VAL_Serial_Printf("Failed to start ADC DMA\r\n");
    return VAL_ERROR;
  }
  
  return VAL_OK;
}

/**
  * @brief  Start a new ADC conversion sequence
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_StartConversion(void) {
  /* Reset conversion complete flag */
  conversionComplete = 0;
  
  /* Start ADC conversion */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcBuffer, ADC_BUFFER_SIZE) != HAL_OK) {
    return VAL_ERROR;
  }
  
  return VAL_OK;
}

/**
  * @brief  Check if ADC conversion is complete
  * @retval uint8_t: 1 if complete, 0 if ongoing
  */
uint8_t VAL_Analog_IsConversionComplete(void) {
  return conversionComplete;
}


VAL_Status VAL_Analog_DiagnosticReadRaw(void) {
    HAL_StatusTypeDef status;
    uint32_t adcBufferLocal[ADC_BUFFER_SIZE];
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Stop any ongoing conversions */
    status = HAL_ADC_Stop(&hadc1);
    if (status != HAL_OK) {
        VAL_Serial_Printf("Stop ADC Failed: %d\r\n", status);
        return VAL_ERROR;
    }

    /* Reset handle state */
    __HAL_ADC_RESET_HANDLE_STATE(&hadc1);

    /* Reconfigure if needed */
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        VAL_Serial_Printf("ADC Reinit Failed\r\n");
        return VAL_ERROR;
    }

    /* Verify channel configuration */
    VAL_Serial_Printf("ADC Configuration Check:\r\n");

    /* Known channel sequence from initialization */
    uint32_t channels[] = {
        ADC_CHANNEL_6,   // PA1
        ADC_CHANNEL_8,   // PA3
        ADC_CHANNEL_9,   // PA4
        ADC_CHANNEL_10,  // PA5
        ADC_CHANNEL_11,  // PA6
        ADC_CHANNEL_12   // PA7
    };

    /* Manually go through each channel */
    for (int i = 0; i < 6; i++) {
        /* Manually reset configuration */
        sConfig.Channel = channels[i];
        sConfig.Rank = ADC_REGULAR_RANK_1;  // Reset to first rank
        sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
        sConfig.SingleDiff = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset = 0;

        /* Reconfigure channel */
        status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        if (status != HAL_OK) {
            VAL_Serial_Printf("Failed to configure Channel %lu\r\n", channels[i]);
            continue;
        }

        /* Start conversion for this channel */
        status = HAL_ADC_Start(&hadc1);
        if (status != HAL_OK) {
            VAL_Serial_Printf("ADC Start Failed for Channel %lu: %d\r\n", channels[i], status);
            continue;
        }

        /* Poll for conversion */
        status = HAL_ADC_PollForConversion(&hadc1, 100);
        if (status != HAL_OK) {
            VAL_Serial_Printf("ADC Poll Failed for Channel %lu: %d\r\n", channels[i], status);
            HAL_ADC_Stop(&hadc1);
            continue;
        }

        /* Get the conversion result */
        adcBufferLocal[i] = HAL_ADC_GetValue(&hadc1);
        VAL_Serial_Printf("Channel %d (ADC_CHANNEL_%lu): %lu\r\n",
                          i, channels[i], adcBufferLocal[i]);

        /* Stop conversion */
        HAL_ADC_Stop(&hadc1);
    }

    return VAL_OK;
}
/**
  * @brief  Get current reading for a specific light
  * @param  lightId: Light ID (1-3)
  * @param  current: Pointer to store current value in milliamps
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_GetCurrent(uint8_t lightId, float* current) {
  /* Check parameters */
  if (lightId < 1 || lightId > LIGHT_COUNT || current == NULL) {
    return VAL_PARAM;
  }
  
  /* Map light ID to ADC channel */
  AdcChannelIndex channelIndex = CHANNEL_CURRENT_1 + (lightId - 1);
  
  VAL_Analog_DiagnosticReadRaw();

  /* Get raw ADC value */
  uint32_t adcValue = adcBuffer[channelIndex];
  
  /* Convert to voltage */
  float voltage = (adcValue / ADC_RESOLUTION) * ADC_REFERENCE;
  
  /* Convert to current (milliamps) */
  *current = (voltage / CURRENT_SENSE_RATIO) * 1000.0f;
  
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
  uint32_t adcValue = adcBuffer[channelIndex];
  
  /* Convert to voltage */
  float voltage = (adcValue / ADC_RESOLUTION) * ADC_REFERENCE;
  
  /* Calculate resistance (voltage divider with 10k pullup) */
  float resistance = (10000.0f * voltage) / (ADC_REFERENCE - voltage);
  
  /* Convert to temperature using B-parameter equation */
  float tempKelvin = 1.0f / ((1.0f / TEMP_SENSOR_T0) + (1.0f / TEMP_SENSOR_BETA) * logf(resistance / TEMP_SENSOR_R0));
  
  /* Convert to Celsius */
  *temperature = tempKelvin - 273.15f;
  
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
  
  /* Set light ID */
  sensorData->lightId = lightId;
  
  return VAL_OK;
}

/**
  * @brief  Get sensor readings for all lights
  * @param  sensorData: Array to store sensor data (must be at least LIGHT_COUNT elements)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_Analog_GetAllSensorData(LightSensorData sensorData[]) {
  VAL_Status status = VAL_OK;
  
  /* Get sensor data for each light */
  for (uint8_t i = 0; i < LIGHT_COUNT; i++) {
    VAL_Status lightStatus = VAL_Analog_GetSensorData(i + 1, &sensorData[i]);
    if (lightStatus != VAL_OK) {
      status = lightStatus;
    }
  }
  
  return status;
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
  if (hadc->Instance == ADC1) {
    conversionComplete = 1;
  }
}
