/**
  ******************************************************************************
  * @file    app_comms_handler.c
  * @brief   Application layer communications handler module
  ******************************************************************************
  * @attention
  *
  * This module implements a minimal communications handler with ping functionality.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_comms_handler.h"
#include "app_sys_coordinator.h"  // For light intensity retrieval
#include "val.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "lwjson/lwjson.h" /* JSON parser library */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* Private define ------------------------------------------------------------*/
#define COMMS_HANDLER_STACK_SIZE   384
#define COMMS_HANDLER_PRIORITY     osPriorityNormal

#define RX_BUFFER_SIZE             256
#define TX_BUFFER_SIZE             512

/* Message types */
#define MSG_TYPE_CMD               "cmd"
#define MSG_TYPE_RESP              "resp"
#define MSG_TYPE_EVENT             "event"

/* Private variables ---------------------------------------------------------*/
static TaskHandle_t commsHandlerTaskHandle = NULL;
static char rxBuffer[RX_BUFFER_SIZE];
static char txBuffer[TX_BUFFER_SIZE];
static uint16_t rxIndex = 0;

static lwjson_t jsonParser;
static lwjson_token_t jsonTokens[32];

/* Private function prototypes -----------------------------------------------*/
static void COMMS_Handler_Task(void const *argument);
static void COMMS_ProcessJsonCommand(const char* jsonStr);
static void COMMS_SerialRxCallback(uint8_t byte);
static void COMMS_SendPingResponse(const char* msgId);
static void COMMS_SendLightIntensityResponse(const char* msgId, uint8_t lightId);
static void COMMS_SendSetLightResponse(const char* msgId, VAL_Status status);
static void COMMS_SendSetAllLightsResponse(const char* msgId, VAL_Status status);
static void COMMS_SendSensorDataResponse(const char* msgId, uint8_t lightId);
static void COMMS_SendAllSensorDataResponse(const char* msgId);
static void COMMS_SendAlarmClearResponse(const char* msgId, uint8_t lightId, VAL_Status status);
static void COMMS_SendAlarmStatusResponse(const char* msgId);
static void COMMS_SendErrorResponse(const char* msgId, const char* topic, const char* action, const char* message);

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the communications handler
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status COMMS_Handler_Init(void) {
  /* Initialize JSON parser */
  lwjson_init(&jsonParser, jsonTokens, LWJSON_ARRAYSIZE(jsonTokens));

  /* Initialize serial with callback */
  VAL_Status status = VAL_Serial_Init(COMMS_SerialRxCallback);
  if (status != VAL_OK) {
    return status;
  }

  /* Create communications handler task */
  osThreadDef(COMSHandlerTask, COMMS_Handler_Task, COMMS_HANDLER_PRIORITY, 0, COMMS_HANDLER_STACK_SIZE);
  commsHandlerTaskHandle = osThreadCreate(osThread(COMSHandlerTask), NULL);

  if (commsHandlerTaskHandle == NULL) {
    return VAL_ERROR;
  }

  return VAL_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Communications handler task
  * @param  argument: Task argument
  * @retval None
  */
static void COMMS_Handler_Task(void const *argument) {
  /* Initialize */

  /* Task main loop */
  for (;;) {
    /* Wait for events, task is primarily driven by the Serial RX callback */
    osDelay(100);
  }
}

/**
  * @brief  Send ping response
  * @param  msgId: Message ID to respond to
  * @retval None
  */
static void COMMS_SendPingResponse(const char* msgId) {
  /* Format ping response */
  int length = snprintf(txBuffer, TX_BUFFER_SIZE,
    "{"
      "\"type\":\"resp\","
      "\"id\":\"%s\","
      "\"topic\":\"system\","
      "\"action\":\"ping\","
      "\"data\":{"
        "\"status\":\"ok\","
        "\"message\":\"pong\""
      "}"
    "}\r\n",
    msgId);

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send light intensity response
 * @param msgId Original message ID
 * @param lightId Light source ID (or 0 for all)
 * @retval None
 */
static void COMMS_SendLightIntensityResponse(const char* msgId, uint8_t lightId) {
  uint8_t intensities[3] = {0, 0, 0};
  VAL_Status status = VAL_ERROR;

  if (lightId == 0) {
    /* Get all light intensities */
    status = SYS_Coordinator_GetAllLightIntensities(intensities);
  } else if (lightId >= 1 && lightId <= 3) {
    /* Get single light intensity */
    status = SYS_Coordinator_GetLightIntensity(lightId, &intensities[lightId - 1]);
  }

  /* Format response */
  int length;
  if (status == VAL_OK) {
    if (lightId == 0) {
      length = snprintf(txBuffer, TX_BUFFER_SIZE,
        "{"
          "\"type\":\"resp\","
          "\"id\":\"%s\","
          "\"topic\":\"light\","
          "\"action\":\"get_all\","
          "\"data\":{"
            "\"status\":\"ok\","
            "\"intensities\":[%u, %u, %u]"
          "}"
        "}\r\n",
        msgId, intensities[0], intensities[1], intensities[2]);
    } else {
      length = snprintf(txBuffer, TX_BUFFER_SIZE,
        "{"
          "\"type\":\"resp\","
          "\"id\":\"%s\","
          "\"topic\":\"light\","
          "\"action\":\"get\","
          "\"data\":{"
            "\"status\":\"ok\","
            "\"id\":%u,"
            "\"intensity\":%u"
          "}"
        "}\r\n",
        msgId, lightId, intensities[lightId - 1]);
    }
  } else {
    /* Error response */
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"light\","
        "\"action\":\"%s\","
        "\"data\":{"
          "\"status\":\"error\","
          "\"message\":\"Failed to retrieve light intensity\""
        "}"
      "}\r\n",
      msgId,
      (lightId == 0) ? "get_all" : "get");
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send response for set light command
 * @param msgId Original message ID
 * @param status Operation status
 * @retval None
 */
static void COMMS_SendSetLightResponse(const char* msgId, VAL_Status status) {
  int length;

  if (status == VAL_OK) {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"light\","
        "\"action\":\"set\","
        "\"data\":{"
          "\"status\":\"ok\""
        "}"
      "}\r\n",
      msgId);
  } else {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"light\","
        "\"action\":\"set\","
        "\"data\":{"
          "\"status\":\"error\","
          "\"message\":\"Failed to set light intensity\""
        "}"
      "}\r\n",
      msgId);
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send response for set all lights command
 * @param msgId Original message ID
 * @param status Operation status
 * @retval None
 */
static void COMMS_SendSetAllLightsResponse(const char* msgId, VAL_Status status) {
  int length;

  if (status == VAL_OK) {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"light\","
        "\"action\":\"set_all\","
        "\"data\":{"
          "\"status\":\"ok\""
        "}"
      "}\r\n",
      msgId);
  } else {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"light\","
        "\"action\":\"set_all\","
        "\"data\":{"
          "\"status\":\"error\","
          "\"message\":\"Failed to set light intensities\""
        "}"
      "}\r\n",
      msgId);
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send sensor data response for a specific light
 * @param msgId Original message ID
 * @param lightId Light source ID
 * @retval None
 */
static void COMMS_SendSensorDataResponse(const char* msgId, uint8_t lightId) {
  LightSensorData_t sensorData;
  VAL_Status status = VAL_ERROR;

  /* Get sensor data for the specified light */
  if (lightId >= 1 && lightId <= 3) {
    status = SYS_Coordinator_GetLightSensorData(lightId, &sensorData);
  }

  /* Format response */
  int length;
  if (status == VAL_OK) {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"status\","
        "\"action\":\"get_sensors\","
        "\"data\":{"
          "\"status\":\"ok\","
          "\"sensor\":{"
            "\"id\":%u,"
            "\"current\":%.1f,"
            "\"temperature\":%.1f"
          "}"
        "}"
      "}\r\n",
      msgId, lightId, sensorData.current, sensorData.temperature);
  } else {
    /* Error response */
    COMMS_SendErrorResponse(msgId, "status", "get_sensors",
                          "Failed to retrieve sensor data");
    return;
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send sensor data response for all lights
 * @param msgId Original message ID
 * @retval None
 */
static void COMMS_SendAllSensorDataResponse(const char* msgId) {
  LightSensorData_t sensorData[3];
  VAL_Status status;

  /* Get sensor data for all lights */
  status = SYS_Coordinator_GetAllLightSensorData(sensorData);

  /* Format response */
  int length;
  if (status == VAL_OK) {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"status\","
        "\"action\":\"get_all_sensors\","
        "\"data\":{"
          "\"status\":\"ok\","
          "\"sensors\":["
            "{\"id\":1,\"current\":%.1f,\"temperature\":%.1f},"
            "{\"id\":2,\"current\":%.1f,\"temperature\":%.1f},"
            "{\"id\":3,\"current\":%.1f,\"temperature\":%.1f}"
          "]"
        "}"
      "}\r\n",
      msgId,
      sensorData[0].current, sensorData[0].temperature,
      sensorData[1].current, sensorData[1].temperature,
      sensorData[2].current, sensorData[2].temperature);
  } else {
    /* Error response */
    COMMS_SendErrorResponse(msgId, "status", "get_all_sensors",
                          "Failed to retrieve sensor data");
    return;
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send response for clear alarm command
 * @param msgId Original message ID
 * @param lightId Light source ID
 * @param status Operation status
 * @retval None
 */
static void COMMS_SendAlarmClearResponse(const char* msgId, uint8_t lightId, VAL_Status status) {
  int length;

  if (status == VAL_OK) {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"alarm\","
        "\"action\":\"clear\","
        "\"data\":{"
          "\"status\":\"ok\","
          "\"message\":\"Alarm cleared for light %u\""
        "}"
      "}\r\n",
      msgId, lightId);
  } else {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"alarm\","
        "\"action\":\"clear\","
        "\"data\":{"
          "\"status\":\"error\","
          "\"message\":\"Failed to clear alarm for light %u\""
        "}"
      "}\r\n",
      msgId, lightId);
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send alarm status response
 * @param msgId Original message ID
 * @retval None
 */
static void COMMS_SendAlarmStatusResponse(const char* msgId) {
  uint8_t alarms[3] = {0, 0, 0};
  VAL_Status status;

  /* Get alarm status for all lights */
  status = SYS_Coordinator_GetAlarmStatus(alarms);

  /* Format response */
  int length;
  if (status == VAL_OK) {
    length = snprintf(txBuffer, TX_BUFFER_SIZE,
      "{"
        "\"type\":\"resp\","
        "\"id\":\"%s\","
        "\"topic\":\"alarm\","
        "\"action\":\"status\","
        "\"data\":{"
          "\"status\":\"ok\","
          "\"active_alarms\":["
      , msgId);

    /* Add active alarms to the response */
    int alarmCount = 0;

    for (int i = 0; i < 3; i++) {
      if (alarms[i] != 0) {
        /* Format alarm entry */
        if (alarmCount > 0) {
          /* Add comma separator for subsequent entries */
          length += snprintf(txBuffer + length, TX_BUFFER_SIZE - length, ",");
        }

        const char* alarmType = "unknown";
        switch (alarms[i]) {
          case 0:
            alarmType = "none";
            break;
          case 1:
            alarmType = "over_current";
            break;
          case 2:
            alarmType = "over_temperature";
            break;
          default:
            alarmType = "system_error";
            break;
        }

        length += snprintf(txBuffer + length, TX_BUFFER_SIZE - length,
          "{\"light\":%d,\"code\":\"%s\"}", i + 1, alarmType);

        alarmCount++;
      }
    }

    /* Complete the JSON object */
    length += snprintf(txBuffer + length, TX_BUFFER_SIZE - length,
      "]"
        "}"
      "}\r\n");

  } else {
    /* Error response */
    COMMS_SendErrorResponse(msgId, "alarm", "status", "Failed to retrieve alarm status");
    return;
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send a generic error response
 * @param msgId Original message ID
 * @param topic Message topic
 * @param action Message action
 * @param message Error message
 * @retval None
 */
static void COMMS_SendErrorResponse(const char* msgId, const char* topic, const char* action, const char* message) {
  int length = snprintf(txBuffer, TX_BUFFER_SIZE,
    "{"
      "\"type\":\"resp\","
      "\"id\":\"%s\","
      "\"topic\":\"%s\","
      "\"action\":\"%s\","
      "\"data\":{"
        "\"status\":\"error\","
        "\"message\":\"%s\""
      "}"
    "}\r\n",
    msgId, topic, action, message);

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
  * @brief  Process a received JSON command string
  * @param  jsonStr: JSON command string
  * @retval None
  */
static void COMMS_ProcessJsonCommand(const char* jsonStr) {
  /* Parse JSON message */
  lwjson_parse(&jsonParser, jsonStr);

  /* Get first token - this is the root object */
  const lwjson_token_t* root = lwjson_get_first_token(&jsonParser);
  if (root == NULL || root->type != LWJSON_TYPE_OBJECT) {
    lwjson_free(&jsonParser);
    return;
  }

  /* Retrieve message details */
  char msgId[64] = "unknown";
  const char* type = NULL;
  const char* topic = NULL;
  const char* action = NULL;
  size_t type_len = 0, topic_len = 0, action_len = 0;

  /* Extract message details */
  const lwjson_token_t* token = root->u.first_child;
  while (token != NULL) {
    if (token->token_name != NULL) {
      if (strncmp(token->token_name, "id", token->token_name_len) == 0 &&
          token->type == LWJSON_TYPE_STRING) {
        size_t id_len;
        const char* tmp = lwjson_get_val_string(token, &id_len);
        if (tmp != NULL) {
          strncpy(msgId, tmp, id_len);
          msgId[id_len] = '\0';
        }
      } else if (strncmp(token->token_name, "type", token->token_name_len) == 0 &&
                 token->type == LWJSON_TYPE_STRING) {
        type = lwjson_get_val_string(token, &type_len);
      } else if (strncmp(token->token_name, "topic", token->token_name_len) == 0 &&
                 token->type == LWJSON_TYPE_STRING) {
        topic = lwjson_get_val_string(token, &topic_len);
      } else if (strncmp(token->token_name, "action", token->token_name_len) == 0 &&
                 token->type == LWJSON_TYPE_STRING) {
        action = lwjson_get_val_string(token, &action_len);
      }
    }
    token = token->next;
  }

  /* Check if this is a valid command */
  if (type != NULL && topic != NULL && action != NULL &&
      strncmp(type, MSG_TYPE_CMD, type_len) == 0) {

    /* Check for system ping command */
    if (topic_len == 6 && strncmp(topic, "system", 6) == 0 &&
        action_len == 4 && strncmp(action, "ping", 4) == 0) {
      /* Send ping response */
      COMMS_SendPingResponse(msgId);
    }
    /* Check for light topic commands */
    else if (topic_len == 5 && strncmp(topic, "light", 5) == 0) {
      if (action_len == 3 && strncmp(action, "get", 3) == 0) {
        /* Check for light ID in data */
        uint8_t lightId = 0;
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "id" inside data */
            const lwjson_token_t* dataToken = token->u.first_child;
            while (dataToken != NULL) {
              if (dataToken->token_name != NULL &&
                  strncmp(dataToken->token_name, "id", dataToken->token_name_len) == 0 &&
                  dataToken->type == LWJSON_TYPE_NUM_INT) {

                lightId = (uint8_t)dataToken->u.num_int;
                break;
              }
              dataToken = dataToken->next;
            }
            break;
          }
          token = token->next;
        }

        /* Send response */
        COMMS_SendLightIntensityResponse(msgId, lightId);
      }
      else if (action_len == 7 && strncmp(action, "get_all", 7) == 0) {
        /* Get intensities for all lights */
        COMMS_SendLightIntensityResponse(msgId, 0);
      }
      else if (action_len == 3 && strncmp(action, "set", 3) == 0) {
        /* Set light intensity */
        uint8_t lightId = 0;
        uint8_t intensity = 0;
        bool foundId = false;
        bool foundIntensity = false;

        /* Find data object */
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "id" and "intensity" inside data */
            const lwjson_token_t* dataToken = token->u.first_child;
            while (dataToken != NULL) {
              if (dataToken->token_name != NULL) {
                if (strncmp(dataToken->token_name, "id", dataToken->token_name_len) == 0 &&
                    dataToken->type == LWJSON_TYPE_NUM_INT) {
                  lightId = (uint8_t)dataToken->u.num_int;
                  foundId = true;
                }
                else if (strncmp(dataToken->token_name, "intensity", dataToken->token_name_len) == 0 &&
                         dataToken->type == LWJSON_TYPE_NUM_INT) {
                  intensity = (uint8_t)dataToken->u.num_int;
                  foundIntensity = true;
                }
              }
              dataToken = dataToken->next;
            }
            break;
          }
          token = token->next;
        }

        /* Set light intensity if parameters are valid */
        VAL_Status status = VAL_ERROR;
        if (foundId && foundIntensity) {
          status = SYS_Coordinator_SetLightIntensity(lightId, intensity);
        }

        /* Send response */
        COMMS_SendSetLightResponse(msgId, status);
      }
      else if (action_len == 7 && strncmp(action, "set_all", 7) == 0) {
        /* Set all light intensities */
        uint8_t intensities[3] = {0, 0, 0};
        bool foundIntensities = false;

        /* Find data object */
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "intensities" array inside data */
            const lwjson_token_t* dataToken = token->u.first_child;
            while (dataToken != NULL) {
              if (dataToken->token_name != NULL &&
                  strncmp(dataToken->token_name, "intensities", dataToken->token_name_len) == 0 &&
                  dataToken->type == LWJSON_TYPE_ARRAY) {

                /* Parse intensities array */
                const lwjson_token_t* arrayToken = dataToken->u.first_child;
                int index = 0;

                while (arrayToken != NULL && index < 3) {
                  if (arrayToken->type == LWJSON_TYPE_NUM_INT) {
                    intensities[index++] = (uint8_t)arrayToken->u.num_int;
                  }
                  arrayToken = arrayToken->next;
                }

                if (index == 3) {
                  foundIntensities = true;
                }
                break;
              }
              dataToken = dataToken->next;
            }
            break;
          }
          token = token->next;
        }

        /* Set all light intensities if parameters are valid */
        VAL_Status status = VAL_ERROR;
        if (foundIntensities) {
          status = SYS_Coordinator_SetAllLightIntensities(intensities);
        }

        /* Send response */
        COMMS_SendSetAllLightsResponse(msgId, status);
      }
    }
    /* Check for status topic commands */
    else if (topic_len == 6 && strncmp(topic, "status", 6) == 0) {
      if (action_len == 11 && strncmp(action, "get_sensors", 11) == 0) {
        /* Get sensor data for a specific light */
        uint8_t lightId = 0;
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "id" inside data */
            const lwjson_token_t* dataToken = token->u.first_child;
            while (dataToken != NULL) {
              if (dataToken->token_name != NULL &&
                  strncmp(dataToken->token_name, "id", dataToken->token_name_len) == 0 &&
                  dataToken->type == LWJSON_TYPE_NUM_INT) {

                lightId = (uint8_t)dataToken->u.num_int;
                break;
              }
              dataToken = dataToken->next;
            }
            break;
          }
          token = token->next;
        }

        if (lightId >= 1 && lightId <= 3) {
          /* Send sensor data response for specific light */
          COMMS_SendSensorDataResponse(msgId, lightId);
        } else {
          /* Invalid light ID */
          COMMS_SendErrorResponse(msgId, "status", "get_sensors", "Invalid light ID");
        }
      }
      else if (action_len == 15 && strncmp(action, "get_all_sensors", 15) == 0) {
        /* Get sensor data for all lights */
        COMMS_SendAllSensorDataResponse(msgId);
      }
    }
    /* Check for alarm topic commands */
    else if (topic_len == 5 && strncmp(topic, "alarm", 5) == 0) {
      if (action_len == 5 && strncmp(action, "clear", 5) == 0) {
        /* Clear alarm for specific light(s) */
        uint8_t lightId = 0;
        bool singleLight = false;
        bool validCommand = false;

        /* Find data object */
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Check for single light ID or array of light IDs */
            const lwjson_token_t* dataToken = token->u.first_child;
            while (dataToken != NULL) {
              if (dataToken->token_name != NULL) {
                /* Single light ID */
                if (strncmp(dataToken->token_name, "id", dataToken->token_name_len) == 0 &&
                    dataToken->type == LWJSON_TYPE_NUM_INT) {
                  lightId = (uint8_t)dataToken->u.num_int;
                  singleLight = true;
                  validCommand = true;
                  break;
                }
                /* Array of light IDs - only handle first one for this implementation */
                else if (strncmp(dataToken->token_name, "lights", dataToken->token_name_len) == 0 &&
                         dataToken->type == LWJSON_TYPE_ARRAY) {
                  const lwjson_token_t* arrayToken = dataToken->u.first_child;
                  if (arrayToken != NULL && arrayToken->type == LWJSON_TYPE_NUM_INT) {
                    lightId = (uint8_t)arrayToken->u.num_int;
                    singleLight = true;
                    validCommand = true;
                    break;
                  }
                }
              }
              dataToken = dataToken->next;
            }
            break;
          }
          token = token->next;
        }

        if (validCommand && singleLight) {
          /* Clear alarm for a specific light */
          VAL_Status status = SYS_Coordinator_ClearLightAlarm(lightId);
          COMMS_SendAlarmClearResponse(msgId, lightId, status);
        } else {
          /* Invalid command */
          COMMS_SendErrorResponse(msgId, "alarm", "clear", "Invalid parameters");
        }
      }
      else if (action_len == 6 && strncmp(action, "status", 6) == 0) {
    	/* Get alarm status for all lights */
        COMMS_SendAlarmStatusResponse(msgId);
      }
    }
  }

  /* Cleanup parser */
  lwjson_free(&jsonParser);
}
/**
  * @brief  Serial RX callback - Called for each byte received
  * @param  byte: Received byte
  * @retval None
  */
static void COMMS_SerialRxCallback(uint8_t byte) {
  /* Add byte to buffer */
  if (rxIndex < RX_BUFFER_SIZE - 1) {
    rxBuffer[rxIndex++] = byte;
  }

  /* Check for end of message */
  if (byte == '\n' || byte == '\r') {
    /* Null-terminate the string */
    rxBuffer[rxIndex] = '\0';

    /* Check if we have a non-empty message */
    if (rxIndex > 0) {
      /* Process the received message */
      COMMS_ProcessJsonCommand(rxBuffer);
    }

    /* Reset index for next message */
    rxIndex = 0;
  }
}
