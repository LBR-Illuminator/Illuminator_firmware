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

  /* Iterate through all tokens in the root object */
  const lwjson_token_t* token = root->u.first_child;

  /* Look for type field */
  while (token != NULL) {
    /* Check if token is "type" */
    if (token->token_name != NULL &&
        strncmp(token->token_name, "type", token->token_name_len) == 0 &&
        token->type == LWJSON_TYPE_STRING) {

      size_t type_len;
      const char* type_str = lwjson_get_val_string(token, &type_len);

      /* If type is not "cmd", skip */
      if (type_str == NULL || strncmp(type_str, MSG_TYPE_CMD, type_len) != 0) {
        break;
      }
    }
    token = token->next;
  }

  /* Get message ID, topic, and action */
  const char* msgId = "unknown";
  const char* topic = NULL;
  const char* action = NULL;
  size_t topic_len = 0, action_len = 0;

  token = root->u.first_child;
  while (token != NULL) {
    if (token->token_name != NULL) {
      if (strncmp(token->token_name, "id", token->token_name_len) == 0 &&
          token->type == LWJSON_TYPE_STRING) {
        size_t id_len;
        const char* tmp = lwjson_get_val_string(token, &id_len);
        if (tmp != NULL) {
          msgId = tmp;
        }
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

  /* Check if this is a system ping command */
  if (topic != NULL && action != NULL &&
      topic_len == 6 && strncmp(topic, "system", 6) == 0 &&
      action_len == 4 && strncmp(action, "ping", 4) == 0) {
    /* Send ping response */
    COMMS_SendPingResponse(msgId);
  }

  /* Cleanup parser */
  lwjson_free(&jsonParser);
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
      //VAL_Serial_Printf("RX Callback: Full message received: %s\r\n", rxBuffer);

      /* Process the received message */
      COMMS_ProcessJsonCommand(rxBuffer);
    }

    /* Reset index for next message */
    rxIndex = 0;
  }
}
