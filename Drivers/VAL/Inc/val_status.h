/**
  ******************************************************************************
  * @file    val_status.h
  * @brief   Common status definitions for VAL modules
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VAL_STATUS_H
#define __VAL_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
typedef enum {
  VAL_OK       = 0x00,  /* Operation successful */
  VAL_ERROR    = 0x01,  /* Operation failed */
  VAL_BUSY     = 0x02,  /* Resource is busy */
  VAL_TIMEOUT  = 0x03,  /* Operation timed out */
  VAL_PARAM    = 0x04,  /* Invalid parameter */
} VAL_Status;

#ifdef __cplusplus
}
#endif

#endif /* __VAL_STATUS_H */
