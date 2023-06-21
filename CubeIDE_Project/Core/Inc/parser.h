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
#include "stdbool.h"			// For type bool
#include "printf.h"				// For optimized printf function made for embedded systems
#include "string.h"				// For string manipulation functions
#include "helper.h"				// For various helper functions
#include "stdbool.h"			// For bool datatype

// ------------------------------------------------------------ MACROS
// Module macros
#define PARSER_MODULE_STACK_SIZE			512
#define PARSER_MODULE_NAME					"PARSER"
#define PARSER_MODULE_INIT_NAME				"ParserInit"
#define PARSER_RETVAL						uint8_t
// Status macros (0x0X)
#define PARSER_OK							0x00
#define PARSER_FAILED						0x01
// Error macros (0xFX)
#define PARSER_NOT_IMPLEMENTED 				0xFA
#define PARSER_MEM_ERROR					0xFB
#define PARSER_RESERVED_ERROR				0xFC
#define PARSER_DATA_ERROR					0xFD
#define PARSER_SEMAPHORE_ERROR				0xFE
#define PARSER_INIT_ERROR					0xFF

// ------------------------------------------------------------ TYPES
// Module type definitions
typedef struct parser_module {
	uint8_t module_flags;
} parser_module_t;

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
void StartParser(void *argument);
void Parser(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
PARSER_RETVAL parser_handle_input();
PARSER_RETVAL parser_handle_message();
PARSER_RETVAL parser_handle_copyReattempt();
PARSER_RETVAL parser_parseInput();
PARSER_RETVAL parser_parseMessage();
bool parser_failureHandler(PARSER_RETVAL value);

#endif /* INC_PARSER_H_ */
