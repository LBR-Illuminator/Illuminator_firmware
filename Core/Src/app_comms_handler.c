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

/* Private define ------------------------------------------------------------*/
#define COMMS_HANDLER_STACK_SIZE   384
#define COMMS_HANDLER_PRIORITY     osPriorityNormal

#define RX_BUFFER_SIZE             256
#define TX_BUFFER_SIZE             256

/* Message types */
#define MSG_TYPE_CMD               "cmd"
#define MSG_TYPE_RESP              "resp"

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
