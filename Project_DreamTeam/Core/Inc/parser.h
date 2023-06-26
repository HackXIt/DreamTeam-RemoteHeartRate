/*
 * parser.h
 *
 *  Created on: Jun 19, 2023
 *      Author: rini
 */

#ifndef INC_PARSER_H_
#define INC_PARSER_H_

// ------------------------------------------------------------ IMPORTS
#include "application.h"		// For application wide definitions
#include "FreeRTOS.h"			// For FreeRTOS API functions (and general functionality)
#include "cmsis_os2.h"			// For CMSIS API functions
#include "usart.h"				// For UART handles and lowLevel access
#include "printf.h"				// For optimized printf function made for embedded systems
#include "string.h"				// For string manipulation functions
#include "helper.h"				// For various helper functions
#include "stdbool.h"			// For bool datatype
#include "stdio.h"				// For sscanf()
//#include "picohttpparser.h"		// For parsing HTTP requests

// ------------------------------------------------------------ MACROS
// Module macros
#define PARSER_MODULE_STACK_SIZE			512
#define PARSER_REQUEST_BUFFER_SIZE			256
#define PARSER_RESPONSE_BUFFER_SIZE			256
#define PARSER_MODULE_NAME					"PARSER"
#define PARSER_MODULE_INIT_NAME				"ParserInit"
#define PARSER_RETVAL						uint8_t
// Status macros (0x0X)
#define PARSER_OK							APPLICATION_OK
#define PARSER_FAILED						0x01
#define PARSER_FAILED_SEMAPHORE				0x02
#define PARSER_NEW_REQUEST					0x04
// Error macros (0xFX)
#define PARSER_NOT_IMPLEMENTED 				0xFA
#define PARSER_MEM_ERROR					0xFB
#define PARSER_RESERVED_ERROR				0xFC
#define PARSER_DATA_ERROR					0xFD
#define PARSER_RESOURCE_ERROR				0xFE
#define PARSER_INIT_ERROR					0xFF

// ------------------------------------------------------------ TYPES
// Module type definitions
/**
 * @brief A structure to provide flags for behaviour change of the module
 * @note Currently there are no behaviour changes implemented.
 */
typedef struct parser_module {
	uint8_t module_flags;
} parser_module_t;

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
/**
 * @brief An initialization task for this module
 *
 * @param argument ... provide @ref parser_module_t for module behaviour change
 */
void StartParser(void *argument);

/**
 * @brief Main task for this module
 *
 * @param argument ... provide @ref parser_module_t for module behaviour change
 */
void Parser(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks

/**
 * @brief On @ref NEW_INPUT signal, parses given input and notifies WIFIBLE module
 *
 * @return either error or status macros, @ref PARSER_OK on success
 */
PARSER_RETVAL parser_handle_input();

/**
 * @brief On @ref NEW_MESSAGE signal, parses given newMsg and notifies WIFIBLE module
 * @note Behaviour varies, depending on message received
 *
 * @return either error or status macros, @ref PARSER_OK on success
 */
PARSER_RETVAL parser_handle_message();

/**
 * @brief On @ref COPY_REATTEMPT signal, rethrows notification to WIFIBLE module for re-attempting
 * @note Not fully implemented, but was supposed to handle reattempt on SEMAPHORE errors when copying data into global
 *
 * @return either error or status macros, @ref PARSER_OK on success
 */
PARSER_RETVAL parser_handle_copyReattempt();

/**
 * @brief Parses the global variable '@ref input'
 *
 * @return either error or status macros, PARSER_OK on success
 */
PARSER_RETVAL parser_parseInput();

/**
 * @brief Parses the global variable '@ref newMsg'
 *
 * @return either error or status macros, @ref PARSER_OK on success
 */
PARSER_RETVAL parser_parseMessage();

/**
 * @brief Handles errors from parser module functions. Serves as an error-handling wrapper for conditions
 *
 * @param value ... return value of any PARSER module function
 * @return true on failure values, false on successful values
 */
bool parser_failureHandler(PARSER_RETVAL value);

#endif /* INC_PARSER_H_ */
