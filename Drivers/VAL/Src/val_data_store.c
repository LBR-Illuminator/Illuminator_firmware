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

