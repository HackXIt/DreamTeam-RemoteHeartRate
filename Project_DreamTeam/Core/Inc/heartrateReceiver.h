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
/**
 * @brief A structure to provide flags for behaviour change of the module
 * @note Currently there are no behaviour changes implemented.
 */
typedef struct hrReceiver_module {
	uint8_t module_flags;
} hrReceiver_module_t;

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
/**
 * @brief An initialization task for this module
 *
 * @param argument ... provide @ref hrReceiver_module_t for module behaviour change
 */
void StartHrReceiver(void *argument);

/**
 * @brief Main task for this module
 *
 * @param argument ... provide @ref hrReceiver_module_t for module behaviour change
 */
void HrReceiver(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
/**
 * @brief On @ref I2C_EVENT signal, processes the heartrate values
 * @note There currently isn't any processing necessary, but the function can output debug functionality
 *
 * @return either error or status macros, @ref HRRECEIVER_OK on success
 */
HRRECEIVER_RETVAL hrReceiver_handle_heartrate();

/**
 * @brief Prepares peripherals used in this module. Module fails to function without it.
 *
 * @return either error or status macros, @ref HRRECEIVER_OK on success
 */
HRRECEIVER_RETVAL hrReceiver_prep_peripherals();

/**
 * @brief handles errors from console module functions. Serves as an error-handling wrapper for conditions
 *
 * @param value ... return value of any HRRECEIVER module function
 * @return true on failure values, false on successful values
 */
bool hrReceiver_failureHandler(HRRECEIVER_RETVAL value);

#endif /* INC_HEARTRATERECEIVER_H_ */
