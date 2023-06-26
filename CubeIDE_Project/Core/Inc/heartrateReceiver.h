/*
 * heartrateReceiver.h
 *
 *  Created on: Jun 23, 2023
 *      Author: rini
 */

#ifndef INC_HEARTRATERECEIVER_H_
#define INC_HEARTRATERECEIVER_H_

// ------------------------------------------------------------ IMPORTS
#include "application.h"		// For application wide definitions
#include "FreeRTOS.h"			// For FreeRTOS API functions (and general functionality)
#include "cmsis_os2.h"			// For CMSIS API functions
#include "i2c.h"				// For UART handles and lowLevel access
#include "stm32l4xx_ll_i2c.h"	// For lowLevel i2c functions
#include "stdbool.h"			// For type bool
#include "printf.h"				// For optimized printf function made for embedded systems
#include "string.h"				// For string manipulation functions
#include "helper.h"				// For various helper functions
#include "stdio.h"				// For sscanf()

// ------------------------------------------------------------ MACROS
// Module macros
#define HRRECEIVER_MODULE_STACK_SIZE			512
#define HRRECEIVER_MODULE_NAME					"hrRECEIVER"
#define HRRECEIVER_MODULE_INIT_NAME				"hrReceiverInit"
#define HRRECEIVER_RETVAL						uint8_t
// Status macros (0x0X)
#define HRRECEIVER_OK							APPLICATION_OK
#define HRRECEIVER_FAILED						0x01
// Error macros (0xFX)
#define HRRECEIVER_RESOURCE_ERROR				0xFE
#define HRRECEIVER_INIT_ERROR					0xFF

// ------------------------------------------------------------ TYPES
// Module type definitions
typedef struct hrReceiver_module {
	uint8_t module_flags;
} hrReceiver_module_t;

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
void StartHrReceiver(void *argument);
void HrReceiver(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
HRRECEIVER_RETVAL hrReceiver_handle_heartrate();
HRRECEIVER_RETVAL hrReceiver_prep_peripherals();
bool hrReceiver_failureHandler(HRRECEIVER_RETVAL value);
void hrReceiver_translate_I2CError(uint32_t error_value);

#endif /* INC_HEARTRATERECEIVER_H_ */
