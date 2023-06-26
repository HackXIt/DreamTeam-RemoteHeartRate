/*
 * console.h
 *
 *  Created on: May 30, 2023
 *      Author: rini
 */

#ifndef INC_CONSOLE_H_
#define INC_CONSOLE_H_

// ------------------------------------------------------------ IMPORTS
#include <application.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "usart.h"
#include "printf.h"
#include "string.h"
#include "helper.h"

// ------------------------------------------------------------ MACROS
// NOTE: I tried to pick hex-values that are memorable for the purpose
// The highest 4 bits represent the category, the lower 4 bits represent the state
// Module macros
#define CONSOLE_MODULE_STACK_SIZE			512
#define CONSOLE_MODULE_NAME					"CONSOLE"
#define CONSOLE_MODULE_INIT_NAME			"ConsoleInit"
#define CONSOLE_RETVAL						uint8_t
#define CONSOLE_UART_BUFFER_SIZE			128
// Status macros (0x0X)
#define CONSOLE_OK							0x00
#define CONSOLE_FAILED_COPY					0x01
#define CONSOLE_FAILED_SEMAPHORE			0x02
#define CONSOLE_FAILED_FLAG					0x04
#define CONSOLE_NEW_DATA					0x08
// Command macros (0xCX) => Not in use
#define CONSOLE_NEW_COMMAND					0xC0
// Data macros (0xDX) => Not in use
// Enable macros (0xEX) => Not in use
#define FEEDBACK_ON							0xE1
#define PREFIX_ON							0xE2
#define TIMESTAMP_ON						0xE4
#define INPUT_ON							0xE8
// Error macros (0xFX)
#define CONSOLE_NOT_IMPLEMENTED 			0xFA
#define CONSOLE_MEM_ERROR					0xFB
#define CONSOLE_RESOURCE_ERROR				0xFC
#define CONSOLE_RW_ERROR					0xFD
#define CONSOLE_ENABLE_ERROR				0xFE
#define CONSOLE_INIT_ERROR					0xFF

// ------------------------------------------------------------ TYPES

// Module type definitions
/**
 * @brief A structure to provide flags for behaviour change of the module
 * @note Currently there are no behaviour changes implemented.
 */
typedef struct console_module {
	uint8_t module_flags;
} console_module_t;

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
/**
 * @brief An initialization task for this module
 *
 * @param argument ... provide console_module_t for module behaviour change
 */
void StartConsoleInterface(void *argument);

/**
 * @brief Main task for this module
 *
 * @param argument ... provide console_module_t for module behaviour change
 */
void ConsoleMonitor(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
/**
 * @brief On @ref RX_BYTE signal, handles a received byte from the serial console (USART2)
 *
 * @return either error or status macros, CONSOLE_OK on success
 */
CONSOLE_RETVAL console_handle_rxByte();

/**
 * @brief enters the memory copy function and also clears previously allocated memory
 *
 * @return either error or status macros, @ref CONSOLE_OK on success
 */
CONSOLE_RETVAL console_enter_copy();

/**
 * @brief On @ref NEW_INPUT signal, processes a given user input and forward a copy to WIFIBLE module
 *
 * @return either error or status macros, @ref CONSOLE_OK on success
 */
CONSOLE_RETVAL console_handle_newInput();

/**
 * @brief On @ref BUFFER_OVERRUN signal, all entered characters are deleted and user is informed
 *
 * @return either error or status macros, @ref CONSOLE_OK on success
 */
CONSOLE_RETVAL console_handle_bufferOverrun();

/**
 * @brief Prepares peripherals used in this module. Module fails to function without it.
 *
 * @return either error or status macros, @ref CONSOLE_OK on success
 */
CONSOLE_RETVAL console_prep_peripherals();

/**
 * @brief Clears the current buffer for USART2 and sets everything to NUL-terminating characters
 *
 * @return either error or status macros, @ref CONSOLE_OK on success
 */
CONSOLE_RETVAL console_clear_buffer();

/**
 * @brief handles errors from console module functions. Serves as an error-handling wrapper for conditions
 *
 * @param value ... return value of any CONSOLE module function
 * @return true on failure values, false on successful values
 */
bool console_failureHandler(CONSOLE_RETVAL value);

#endif /* INC_CONSOLE_H_ */