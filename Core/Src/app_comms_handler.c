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
static TaskHandle_t comms_handler_task_handle = NULL;
static char rxBuffer[RX_BUFFER_SIZE];
static char txBuffer[TX_BUFFER_SIZE];
static uint16_t rx_index = 0;

static lwjson_t jsonParser;
static lwjson_token_t jsonTokens[32];

/* Private function prototypes -----------------------------------------------*/
static void COMMS_Handler_Task(void const *argument);
static void COMMS_Handler_ProcessJsonCommand(const char* json_str);
static void COMMS_Handler_SerialRxCallback(uint8_t byte);
static void COMMS_Handler_SendPingResponse(const char* msg_id);
static void COMMS_Handler_SendLightIntensityResponse(const char* msg_id, uint8_t light_id);
static void COMMS_Handler_SendSetLightResponse(const char* msg_id, VAL_Status status);
static void COMMS_Handler_SendSetAllLightsResponse(const char* msg_id, VAL_Status status);
static void COMMS_Handler_SendSensorDataResponse(const char* msg_id, uint8_t light_id);
static void COMMS_Handler_SendAllSensorDataResponse(const char* msg_id);
static void COMMS_Handler_SendAlarmClearResponse(const char* msg_id, uint8_t light_id, VAL_Status status);
static void COMMS_Handler_SendAlarmStatusResponse(const char* msg_id);
static void COMMS_Handler_SendErrorResponse(const char* msg_id, const char* topic, const char* action, const char* message);

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the communications handler
  * @retval VAL_Status: VAL_OK if successful, VAL_ERROR otherwise
  */
VAL_Status COMMS_Handler_Handler_Init(void) {
  /* Initialize JSON parser */
  lwjson_init(&jsonParser, jsonTokens, LWJSON_ARRAYSIZE(jsonTokens));

  /* Initialize serial with callback */
  VAL_Status status = VAL_Serial_Init(COMMS_Handler_SerialRxCallback);
  if (status != VAL_OK) {
    return status;
  }

  /* Create communications handler task */
  osThreadDef(COMSHandlerTask, COMMS_Handler_Task, COMMS_HANDLER_PRIORITY, 0, COMMS_HANDLER_STACK_SIZE);
  comms_handler_task_handle = osThreadCreate(osThread(COMSHandlerTask), NULL);

  if (comms_handler_task_handle == NULL) {
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
static void COMMS_Handler_SendPingResponse(const char* msg_id) {
  /* Format ping response */
  uint16_t length = snprintf(txBuffer, TX_BUFFER_SIZE,
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
    msg_id);

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send light intensity response
 * @param msgId Original message ID
 * @param light_id Light source ID (or 0 for all)
 * @retval None
 */
static void COMMS_Handler_SendLightIntensityResponse(const char* msg_id, uint8_t light_id) {
  uint8_t intensities[3] = {0, 0, 0};
  VAL_Status status = VAL_ERROR;

  if (light_id == 0) {
    /* Get all light intensities */
    status = SYS_Coordinator_GetAllLightIntensities(intensities);
  } else if (light_id >= 1 && light_id <= 3) {
    /* Get single light intensity */
    status = SYS_Coordinator_GetLightIntensity(light_id, &intensities[light_id - 1]);
  }

  /* Format response */
  uint16_t length;
  if (status == VAL_OK) {
    if (light_id == 0) {
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
        msg_id, intensities[0], intensities[1], intensities[2]);
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
        msg_id, light_id, intensities[light_id - 1]);
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
      msg_id,
      (light_id == 0) ? "get_all" : "get");
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
static void COMMS_Handler_SendSetLightResponse(const char* msg_id, VAL_Status status) {
  uint16_t length;

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
      msg_id);
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
      msg_id);
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
static void COMMS_Handler_SendSetAllLightsResponse(const char* msg_id, VAL_Status status) {
  uint16_t length;

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
      msg_id);
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
      msg_id);
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send sensor data response for a specific light
 * @param msgId Original message ID
 * @param light_id Light source ID
 * @retval None
 */
static void COMMS_Handler_SendSensorDataResponse(const char* msg_id, uint8_t light_id) {
  LightSensorData_t sensor_data;
  VAL_Status status = VAL_ERROR;

  /* Get sensor data for the specified light */
  if (light_id >= 1 && light_id <= 3) {
    status = SYS_Coordinator_GetLightSensorData(light_id, &sensor_data);
  }

  /* Format response */
  uint16_t length;
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
      msg_id, light_id, sensor_data.current, sensor_data.temperature);
  } else {
    /* Error response */
    COMMS_Handler_SendErrorResponse(msg_id, "status", "get_sensors",
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
static void COMMS_Handler_SendAllSensorDataResponse(const char* msg_id) {
  LightSensorData_t sensor_data[3];
  VAL_Status status;

  /* Get sensor data for all lights */
  status = SYS_Coordinator_GetAllLightSensorData(sensor_data);

  /* Format response */
  uint16_t length;
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
      msg_id,
      sensor_data[0].current, sensor_data[0].temperature,
      sensor_data[1].current, sensor_data[1].temperature,
      sensor_data[2].current, sensor_data[2].temperature);
  } else {
    /* Error response */
    COMMS_Handler_SendErrorResponse(msg_id, "status", "get_all_sensors",
                          "Failed to retrieve sensor data");
    return;
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send response for clear alarm command
 * @param msgId Original message ID
 * @param light_id Light source ID
 * @param status Operation status
 * @retval None
 */
static void COMMS_Handler_SendAlarmClearResponse(const char* msg_id, uint8_t light_id, VAL_Status status) {
  uint16_t length;

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
      msg_id, light_id);
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
      msg_id, light_id);
  }

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send alarm status response
 * @param msgId Original message ID
 * @retval None
 */
static void COMMS_Handler_SendAlarmStatusResponse(const char* msg_id) {
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
      , msg_id);

    /* Add active alarms to the response */
    int alarm_count = 0;

    for (int i = 0; i < 3; i++) {
      if (alarms[i] != 0) {
        /* Format alarm entry */
        if (alarm_count > 0) {
          /* Add comma separator for subsequent entries */
          length += snprintf(txBuffer + length, TX_BUFFER_SIZE - length, ",");
        }

        const char* alarm_type = "unknown";
        switch (alarms[i]) {
          case 0:
            alarm_type = "none";
            break;
          case 1:
            alarm_type = "over_current";
            break;
          case 2:
            alarm_type = "over_temperature";
            break;
          default:
            alarm_type = "system_error";
            break;
        }

        length += snprintf(txBuffer + length, TX_BUFFER_SIZE - length,
          "{\"light\":%d,\"code\":\"%s\"}", i + 1, alarm_type);

        alarm_count++;
      }
    }

    /* Complete the JSON object */
    length += snprintf(txBuffer + length, TX_BUFFER_SIZE - length,
      "]"
        "}"
      "}\r\n");

  } else {
    /* Error response */
    COMMS_Handler_SendErrorResponse(msg_id, "alarm", "status", "Failed to retrieve alarm status");
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
static void COMMS_Handler_SendErrorResponse(const char* msg_id, const char* topic, const char* action, const char* message) {
  uint16_t length = snprintf(txBuffer, TX_BUFFER_SIZE,
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
    msg_id, topic, action, message);

  /* Send response */
  VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
 * @brief Send alarm event notification
 * @param light_id Light source ID that triggered the alarm (1-3)
 * @param errorType Type of error that caused the alarm
 * @param value Measured value that caused the alarm
 * @retval VAL_Status VAL_OK if successful, VAL_ERROR otherwise
 */
VAL_Status COMMS_Handler_SendAlarmEvent(uint8_t light_id, uint8_t error_type, float value) {
  const char* error_code_str;
  char event_id[16];
  int length;

  /* Generate event ID with timestamp */
  snprintf(event_id, sizeof(event_id), "evt-%lu", HAL_GetTick());

  /* Convert error type to string */
  switch (error_type) {
    case 1:
      error_code_str = "over_current";
      break;
    case 2:
      error_code_str = "over_temperature";
      break;
    default:
      error_code_str = "system_error";
      break;
  }

  /* Format event message */
  length = snprintf(txBuffer, TX_BUFFER_SIZE,
    "{"
      "\"type\":\"event\","
      "\"id\":\"%s\","
      "\"topic\":\"alarm\","
      "\"action\":\"triggered\","
      "\"data\":{"
        "\"timestamp\":\"%lu\","
        "\"code\":\"%s\","
        "\"source\":\"light_%u\","
        "\"value\":%.1f,"
        "\"status\":\"disabled\""
      "}"
    "}\r\n",
    event_id, HAL_GetTick(), error_code_str, light_id, value);

  /* Send event message */
  return VAL_Serial_Send((uint8_t*)txBuffer, length, 1000);
}

/**
  * @brief  Process a received JSON command string
  * @param  jsonStr: JSON command string
  * @retval None
  */
static void COMMS_Handler_ProcessJsonCommand(const char* json_str) {
  /* Parse JSON message */
  lwjson_parse(&jsonParser, json_str);

  /* Get first token - this is the root object */
  const lwjson_token_t* root = lwjson_get_first_token(&jsonParser);
  if (root == NULL || root->type != LWJSON_TYPE_OBJECT) {
    lwjson_free(&jsonParser);
    return;
  }

  /* Retrieve message details */
  char msg_id[64] = "unknown";
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
          strncpy(msg_id, tmp, id_len);
          msg_id[id_len] = '\0';
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
      COMMS_Handler_SendPingResponse(msg_id);
    }
    /* Check for light topic commands */
    else if (topic_len == 5 && strncmp(topic, "light", 5) == 0) {
      if (action_len == 3 && strncmp(action, "get", 3) == 0) {
        /* Check for light ID in data */
        uint8_t light_id = 0;
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "id" inside data */
            const lwjson_token_t* data_token = token->u.first_child;
            while (data_token != NULL) {
              if (data_token->token_name != NULL &&
                  strncmp(data_token->token_name, "id", data_token->token_name_len) == 0 &&
                  data_token->type == LWJSON_TYPE_NUM_INT) {

                light_id = (uint8_t)data_token->u.num_int;
                break;
              }
              data_token = data_token->next;
            }
            break;
          }
          token = token->next;
        }

        /* Send response */
        COMMS_Handler_SendLightIntensityResponse(msg_id, light_id);
      }
      else if (action_len == 7 && strncmp(action, "get_all", 7) == 0) {
        /* Get intensities for all lights */
        COMMS_Handler_SendLightIntensityResponse(msg_id, 0);
      }
      else if (action_len == 3 && strncmp(action, "set", 3) == 0) {
        /* Set light intensity */
        uint8_t light_id = 0;
        uint8_t intensity = 0;
        bool found_id = false;
        bool found_intensity = false;

        /* Find data object */
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "id" and "intensity" inside data */
            const lwjson_token_t* data_token = token->u.first_child;
            while (data_token != NULL) {
              if (data_token->token_name != NULL) {
                if (strncmp(data_token->token_name, "id", data_token->token_name_len) == 0 &&
                    data_token->type == LWJSON_TYPE_NUM_INT) {
                  light_id = (uint8_t)data_token->u.num_int;
                  found_id = true;
                }
                else if (strncmp(data_token->token_name, "intensity", data_token->token_name_len) == 0 &&
                         data_token->type == LWJSON_TYPE_NUM_INT) {
                  intensity = (uint8_t)data_token->u.num_int;
                  found_intensity = true;
                }
              }
              data_token = data_token->next;
            }
            break;
          }
          token = token->next;
        }

        /* Set light intensity if parameters are valid */
        VAL_Status status = VAL_ERROR;
        if (found_id && found_intensity) {
          status = SYS_Coordinator_SetLightIntensity(light_id, intensity);
        }

        /* Send response */
        COMMS_Handler_SendSetLightResponse(msg_id, status);
      }
      else if (action_len == 7 && strncmp(action, "set_all", 7) == 0) {
        /* Set all light intensities */
        uint8_t intensities[3] = {0, 0, 0};
        bool found_intensities = false;

        /* Find data object */
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "intensities" array inside data */
            const lwjson_token_t* data_token = token->u.first_child;
            while (data_token != NULL) {
              if (data_token->token_name != NULL &&
                  strncmp(data_token->token_name, "intensities", data_token->token_name_len) == 0 &&
                  data_token->type == LWJSON_TYPE_ARRAY) {

                /* Parse intensities array */
                const lwjson_token_t* array_token = data_token->u.first_child;
                int index = 0;

                while (array_token != NULL && index < 3) {
                  if (array_token->type == LWJSON_TYPE_NUM_INT) {
                    intensities[index++] = (uint8_t)array_token->u.num_int;
                  }
                  array_token = array_token->next;
                }

                if (index == 3) {
                  found_intensities = true;
                }
                break;
              }
              data_token = data_token->next;
            }
            break;
          }
          token = token->next;
        }

        /* Set all light intensities if parameters are valid */
        VAL_Status status = VAL_ERROR;
        if (found_intensities) {
          status = SYS_Coordinator_SetAllLightIntensities(intensities);
        }

        /* Send response */
        COMMS_Handler_SendSetAllLightsResponse(msg_id, status);
      }
    }
    /* Check for status topic commands */
    else if (topic_len == 6 && strncmp(topic, "status", 6) == 0) {
      if (action_len == 11 && strncmp(action, "get_sensors", 11) == 0) {
        /* Get sensor data for a specific light */
        uint8_t light_id = 0;
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Search for "id" inside data */
            const lwjson_token_t* data_token = token->u.first_child;
            while (data_token != NULL) {
              if (data_token->token_name != NULL &&
                  strncmp(data_token->token_name, "id", data_token->token_name_len) == 0 &&
                  data_token->type == LWJSON_TYPE_NUM_INT) {

                light_id = (uint8_t)data_token->u.num_int;
                break;
              }
              data_token = data_token->next;
            }
            break;
          }
          token = token->next;
        }

        if (light_id >= 1 && light_id <= 3) {
          /* Send sensor data response for specific light */
          COMMS_Handler_SendSensorDataResponse(msg_id, light_id);
        } else {
          /* Invalid light ID */
          COMMS_Handler_SendErrorResponse(msg_id, "status", "get_sensors", "Invalid light ID");
        }
      }
      else if (action_len == 15 && strncmp(action, "get_all_sensors", 15) == 0) {
        /* Get sensor data for all lights */
        COMMS_Handler_SendAllSensorDataResponse(msg_id);
      }
    }
    /* Check for alarm topic commands */
    else if (topic_len == 5 && strncmp(topic, "alarm", 5) == 0) {
      if (action_len == 5 && strncmp(action, "clear", 5) == 0) {
        /* Clear alarm for specific light(s) */
        uint8_t light_id = 0;
        bool single_light = false;
        bool valid_command = false;

        /* Find data object */
        token = root->u.first_child;
        while (token != NULL) {
          if (token->token_name != NULL &&
              strncmp(token->token_name, "data", token->token_name_len) == 0 &&
              token->type == LWJSON_TYPE_OBJECT) {

            /* Check for single light ID or array of light IDs */
            const lwjson_token_t* data_token = token->u.first_child;
            while (data_token != NULL) {
              if (data_token->token_name != NULL) {
                /* Single light ID */
                if (strncmp(data_token->token_name, "id", data_token->token_name_len) == 0 &&
                    data_token->type == LWJSON_TYPE_NUM_INT) {
                  light_id = (uint8_t)data_token->u.num_int;
                  single_light = true;
                  valid_command = true;
                  break;
                }
                /* Array of light IDs - only handle first one for this implementation */
                else if (strncmp(data_token->token_name, "lights", data_token->token_name_len) == 0 &&
                         data_token->type == LWJSON_TYPE_ARRAY) {
                  const lwjson_token_t* array_token = data_token->u.first_child;
                  if (array_token != NULL && array_token->type == LWJSON_TYPE_NUM_INT) {
                    light_id = (uint8_t)array_token->u.num_int;
                    single_light = true;
                    valid_command = true;
                    break;
                  }
                }
              }
              data_token = data_token->next;
            }
            break;
          }
          token = token->next;
        }

        if (valid_command && single_light) {
          /* Clear alarm for a specific light */
          VAL_Status status = SYS_Coordinator_ClearLightAlarm(light_id);
          COMMS_Handler_SendAlarmClearResponse(msg_id, light_id, status);
        } else {
          /* Invalid command */
          COMMS_Handler_SendErrorResponse(msg_id, "alarm", "clear", "Invalid parameters");
        }
      }
      else if (action_len == 6 && strncmp(action, "status", 6) == 0) {
    	/* Get alarm status for all lights */
        COMMS_Handler_SendAlarmStatusResponse(msg_id);
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
static void COMMS_Handler_SerialRxCallback(uint8_t byte) {
  /* Add byte to buffer */
  if (rx_index < RX_BUFFER_SIZE - 1) {
    rxBuffer[rx_index++] = byte;
  }

  /* Check for end of message */
  if (byte == '\n' || byte == '\r') {
    /* Null-terminate the string */
    rxBuffer[rx_index] = '\0';

    /* Check if we have a non-empty message */
    if (rx_index > 0) {
      /* Process the received message */
      COMMS_Handler_ProcessJsonCommand(rxBuffer);
    }

    /* Reset index for next message */
    rx_index = 0;
  }
}
