/**
  ******************************************************************************
  * @file    val_data_store.c
  * @brief   Vendor Abstraction Layer for data storage
  ******************************************************************************
  * @attention
  *
  * This module provides a hardware-independent interface for data storage
  * used by the Wiseled_LBR system for error logging.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "val_data_store.h"
#include <string.h>
#include "ee.h" /* NimaLTD EEPROM emulation library */

/* Private define ------------------------------------------------------------*/
/* Storage area addresses in emulated EEPROM */
#define STORE_ACTIVE_STATUS_ADDR      0
#define STORE_ERROR_LOG_HEADER_ADDR   32
#define STORE_ERROR_LOG_ENTRIES_ADDR  40
#define STORE_ERROR_LOG_ENTRIES       10

/* Private variables ---------------------------------------------------------*/
static uint8_t initialized = 0;
static StatusLog_t statusLog;
static ErrorLogHeader_t errorLogHeader;
static ErrorLogEntry_t errorLogEntries[STORE_ERROR_LOG_ENTRIES];

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the data storage module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_Init(void) {
  /* Check if already initialized */
  if (initialized) {
    return VAL_OK;
  }
  
  /* Initialize NimaLTD EE library */
  if (!EE_Init(NULL, 0)) {
    return VAL_ERROR;
  }
  
  /* Read status log data */
  EE_Read();
  uint8_t* pData = (uint8_t*)&statusLog;
  memcpy(pData, STORE_ACTIVE_STATUS_ADDR, sizeof(StatusLog_t));
  
  /* Read error log header */
  EE_Read();
  pData = (uint8_t*)&errorLogHeader;
  memcpy(pData, STORE_ERROR_LOG_HEADER_ADDR, sizeof(ErrorLogHeader_t));
  
  /* Read error log entries */
  EE_Read();
  pData = (uint8_t*)errorLogEntries;
  memcpy(pData, STORE_ERROR_LOG_ENTRIES_ADDR, sizeof(ErrorLogEntry_t) * STORE_ERROR_LOG_ENTRIES);
  
  initialized = 1;
  return VAL_OK;
}

/**
  * @brief  Set active error for a specific light
  * @param  lightId: Light ID (1-3)
  * @param  errorType: Type of error
  * @param  value: Measured value that caused the error
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_SetActiveError(uint8_t lightId, ErrorType_t errorType, float value) {
  /* Check parameters */
  if (lightId < 1 || lightId > 3) {
    return VAL_PARAM;
  }
  
  /* Update status log */
  uint8_t lightIndex = lightId - 1;
  
  /* Set error bit for this light */
  statusLog.activeErrors |= (1 << lightIndex);
  
  /* Update error details */
  statusLog.errorTypes[lightIndex] = errorType;
  statusLog.errorValues[lightIndex] = value;
  statusLog.errorTimestamps[lightIndex] = HAL_GetTick(); /* Current system time */
  
  /* Write updated status to storage */
  uint8_t* pData = (uint8_t*)&statusLog;
  memcpy(STORE_ACTIVE_STATUS_ADDR, pData, sizeof(StatusLog_t));
  if (!EE_Write()) {
    return VAL_ERROR;
  }
  
  return VAL_OK;
}

/**
  * @brief  Clear active error for a specific light
  * @param  lightId: Light ID (1-3)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_ClearActiveError(uint8_t lightId) {
  /* Check parameters */
  if (lightId < 1 || lightId > 3) {
    return VAL_PARAM;
  }
  
  /* Update status log */
  uint8_t lightIndex = lightId - 1;
  
  /* Clear error bit for this light */
  statusLog.activeErrors &= ~(1 << lightIndex);
  
  /* Write updated status to storage */
  uint8_t* pData = (uint8_t*)&statusLog;
  memcpy(STORE_ACTIVE_STATUS_ADDR, pData, sizeof(StatusLog_t));
  if (!EE_Write()) {
    return VAL_ERROR;
  }
  
  return VAL_OK;
}

/**
  * @brief  Check if a light has an active error
  * @param  lightId: Light ID (1-3)
  * @param  hasError: Pointer to store result (1 if error active, 0 otherwise)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_HasActiveError(uint8_t lightId, uint8_t* hasError) {
  /* Check parameters */
  if (lightId < 1 || lightId > 3 || hasError == NULL) {
    return VAL_PARAM;
  }
  
  /* Check error bit for this light */
  uint8_t lightIndex = lightId - 1;
  *hasError = (statusLog.activeErrors & (1 << lightIndex)) ? 1 : 0;
  
  return VAL_OK;
}

/**
  * @brief  Get status log
  * @param  status: Pointer to store status log
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_GetStatusLog(StatusLog_t* status) {
  /* Check parameters */
  if (status == NULL) {
    return VAL_PARAM;
  }
  
  /* Copy status log */
  memcpy(status, &statusLog, sizeof(StatusLog_t));
  
  return VAL_OK;
}

/**
  * @brief  Log error event
  * @param  lightId: Light ID (1-3)
  * @param  errorType: Type of error
  * @param  value: Measured value that caused the error
  * @param  action: Action taken (e.g., 1 = disabled light)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_LogErrorEvent(uint8_t lightId, ErrorType_t errorType, float value, uint8_t action) {
  /* Check parameters */
  if (lightId < 1 || lightId > 3) {
    return VAL_PARAM;
  }
  
  /* Create new error log entry */
  ErrorLogEntry_t newEntry;
  newEntry.timestamp = HAL_GetTick();
  newEntry.lightId = lightId;
  newEntry.errorType = errorType;
  newEntry.measuredValue = value;
  newEntry.actionTaken = action;
  
  /* Calculate index for new entry (circular buffer) */
  uint8_t entryIndex = errorLogHeader.nextIndex;
  
  /* Update error log header */
  errorLogHeader.totalErrors++;
  errorLogHeader.nextIndex = (entryIndex + 1) % STORE_ERROR_LOG_ENTRIES;
  
  /* Update error log entries array */
  errorLogEntries[entryIndex] = newEntry;
  
  /* Write updated header and entries to storage */
  uint8_t* pHeaderData = (uint8_t*)&errorLogHeader;
  memcpy(STORE_ERROR_LOG_HEADER_ADDR, pHeaderData, sizeof(ErrorLogHeader_t));
  
  uint8_t* pEntryData = (uint8_t*)&newEntry;
  uint32_t entryAddr = STORE_ERROR_LOG_ENTRIES_ADDR + (entryIndex * sizeof(ErrorLogEntry_t));
  memcpy(entryAddr, pEntryData, sizeof(ErrorLogEntry_t));

  if (!EE_Write()) {
    return VAL_ERROR;
  }
  
  return VAL_OK;
}

/**
  * @brief  Get error logs
  * @param  logs: Array to store error log entries
  * @param  maxLogs: Maximum number of entries to retrieve
  * @param  count: Pointer to store actual number of entries retrieved
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_GetErrorLogs(ErrorLogEntry_t* logs, uint8_t maxLogs, uint8_t* count) {
  /* Check parameters */
  if (logs == NULL || count == NULL) {
    return VAL_PARAM;
  }
  
  /* Calculate number of entries to retrieve */
  uint8_t actualCount = (errorLogHeader.totalErrors < STORE_ERROR_LOG_ENTRIES) ?
                         errorLogHeader.totalErrors : STORE_ERROR_LOG_ENTRIES;
  
  if (actualCount > maxLogs) {
    actualCount = maxLogs;
  }
  
  /* Copy entries in reverse chronological order */
  for (uint8_t i = 0; i < actualCount; i++) {
    int8_t index = (int8_t)errorLogHeader.nextIndex - 1 - i;
    if (index < 0) {
      index += STORE_ERROR_LOG_ENTRIES;
    }
    
    logs[i] = errorLogEntries[index];
  }
  
  *count = actualCount;
  return VAL_OK;
}

/**
  * @brief  Clear error logs
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_ClearErrorLogs(void) {
  /* Reset error log header */
  memset(&errorLogHeader, 0, sizeof(ErrorLogHeader_t));
  
  /* Reset error log entries */
  memset(errorLogEntries, 0, sizeof(ErrorLogEntry_t) * STORE_ERROR_LOG_ENTRIES);
  
  /* Write updated header and entries to storage */
  uint8_t* pHeaderData = (uint8_t*)&errorLogHeader;
  memcpy(STORE_ERROR_LOG_HEADER_ADDR, pHeaderData, sizeof(ErrorLogHeader_t));

  uint8_t* pEntriesData = (uint8_t*)errorLogEntries;
  memcpy(STORE_ERROR_LOG_ENTRIES_ADDR, pEntriesData, sizeof(ErrorLogEntry_t) * STORE_ERROR_LOG_ENTRIES);
  
  if (!EE_Write()) {
    return VAL_ERROR;
  }
  
  return VAL_OK;
}

/**
  * @brief  De-initialize the data storage module
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_DeInit(void) {
  initialized = 0;
  return VAL_OK;
}

/**
  * @brief  Format the emulated EEPROM (erases all data)
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status VAL_DataStore_Format(void) {
  if (!EE_Format()) {
    return VAL_ERROR;
  }
  
  /* Re-initialize with default values */
  memset(&statusLog, 0, sizeof(StatusLog_t));
  memset(&errorLogHeader, 0, sizeof(ErrorLogHeader_t));
  memset(errorLogEntries, 0, sizeof(ErrorLogEntry_t) * STORE_ERROR_LOG_ENTRIES);
  
  /* Write default values to storage */
  uint8_t* pStatusData = (uint8_t*)&statusLog;
  memcpy(STORE_ACTIVE_STATUS_ADDR, pStatusData, sizeof(StatusLog_t));

  uint8_t* pHeaderData = (uint8_t*)&errorLogHeader;
  memcpy(STORE_ERROR_LOG_HEADER_ADDR, pHeaderData, sizeof(ErrorLogHeader_t));
  
  uint8_t* pEntriesData = (uint8_t*)errorLogEntries;
  memcpy(STORE_ERROR_LOG_ENTRIES_ADDR, pEntriesData, sizeof(ErrorLogEntry_t) * STORE_ERROR_LOG_ENTRIES);
  
  if (!EE_Write()) {
    return VAL_ERROR;
  }
  
  return VAL_OK;
}
