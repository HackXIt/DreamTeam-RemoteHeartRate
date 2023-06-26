/*
 * webserver.h
 *
 *  Created on: Jun 19, 2023
 *      Author: rini
 */

#ifndef INC_WEBSERVER_H_
/**
 * @brief THIS MODULE IS NOT USED IN THE PROJECT, PLEASE IGNORE
 * @note It was my failed attempt to use libraries, which would do server operations
 *
 * I failed to find a suitable and small enough library. Time was lacking and so I dropped it.
 */
#define INC_WEBSERVER_H_

// ------------------------------------------------------------ IMPORTS
#include "application.h"		// For application wide definitions
#include "FreeRTOS.h"			// For FreeRTOS API functions (and general functionality)
#include "cmsis_os2.h"			// For CMSIS API functions
#include "stdbool.h"			// For type bool
#include "printf.h"				// Optimized printf function made for embedded systems
#include "string.h"				// For string manipulation functions
#include "helper.h"				// For various helper functions
#include "picohttpparser.h"		// For parsing HTTP requests

// ------------------------------------------------------------ MACROS
// Module macros
#define WEBSERVER_MODULE_STACK_SIZE			256
#define WEBSERVER_MODULE_NAME				"WEBSERVER"
#define WEBSERVER_MODULE_INIT_NAME			"WebserverInit"
#define WEBSERVER_MODULE_SERVER_NAME		"Webserver"
#define WEBSERVER_RETVAL					uint8_t
#define WEBSERVER_MAX_CONNECTIONS			2U
#define WEBSERVER_MAX_HANDLE_QUEUE			10U
// Request types (only permitted one's)
#define GET_REQUEST							"GET"
// Status macros (0x0X)
#define WEBSERVER_OK						0x00
#define WEBSERVER_FAILED_COPY				0x01
#define WEBSERVER_FAILED_SEMAPHORE			0x02
#define WEBSERVER_FAILED_FLAG				0x04
#define WEBSERVER_FAILED_QUEUE				0x08
// Error macros (0xFX)
#define WEBSERVER_NOT_IMPLEMENTED 			0xFA
#define WEBSERVER_MEM_ERROR					0xFB
#define WEBSERVER_CONNECTION_ERROR			0xFC
#define WEBSERVER_REQUEST_ERROR				0xFD
#define WEBSERVER_UNKOWN_ERROR				0xFE
#define WEBSERVER_INIT_ERROR				0xFF

// ------------------------------------------------------------ TYPES
/**
 * @brief A structure to provide flags for behaviour change of the module
 * @note Specify which flags will affect the behavior of the module.
 */
typedef struct webserver_module {
	uint8_t module_flags;
} webserver_module_t;

/**
 * @brief Structure to represent client details
 */
typedef struct client{
	uint8_t link_id;
	uint8_t mac_address[6];
	uint8_t ip_address[4];
} client_t;

/**
 * @brief Structure to represent request packet details
 */
typedef struct requestPacket{
    uint8_t link_id;
    char ip_address[16];
    char request_type[8];
    char request_path[64];
    bool connection_alive;
} requestPacket_t ;

/**
 * @brief Structure to represent response packet details
 */
typedef struct responsePacket{
    uint8_t link_id;
    char ip_address[16];
    char request_type[8];
    char response_path[64];
    bool connection_alive;
} responsePacket_t ;

// ------------------------------------------------------------ STATIC VARIABLES
// To-be-defined if any...

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
/**
 * @brief An initialization task for this module
 *
 * @param argument ... provide @ref webserver_module_t for module behaviour change
 */
void StartWebserverModule(void *argument);
/**
 * @brief Main task for this module
 *
 * @param argument ... provide @ref webserver_module_t for module behaviour change
 */
void Webserver(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
/**
 * @brief Handles a new web server connection
 *
 * @return either error or status macros, @ref WEBSERVER_OK on success
 */
WEBSERVER_RETVAL webserver_handle_connection();
/**
 * @brief Handles a new web server request
 *
 * @return either error or status macros, @ref WEBSERVER_OK on success
 */
WEBSERVER_RETVAL webserver_handle_request();
/**
 * @brief Handles errors from webserver module functions. Serves as an error-handling wrapper for conditions
 *
 * @param value ... return value of any WEBSERVER module function
 * @return true on failure values, false on successful values
 */
bool webserver_failureHandler(WEBSERVER_RETVAL value);

// ------------------------------------------------------------ Webserver functions

#endif /* INC_WEBSERVER_H_ */
