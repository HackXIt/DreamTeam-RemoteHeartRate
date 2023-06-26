/*
 * wifi-ble.h
 *
 *  Created on: May 22, 2023
 *      Author: rini
 */

#ifndef INC_WIFI_BLE_H_
#define INC_WIFI_BLE_H_

// ------------------------------------------------------------ IMPORTS
#include "application.h"		// For application wide definitions
#include "FreeRTOS.h"			// For FreeRTOS API functions (and general functionality)
#include "cmsis_os2.h"			// For CMSIS API functions
#include "usart.h"				// For UART handles and lowLevel access
#include "ATcommands.h"			// My personal printf-wrapper-functions for AT-Commands, including macros
#include "stdbool.h"			// For type bool
#include "printf.h"				// For optimized printf function made for embedded systems
#include "string.h"				// For string manipulation functions
#include "helper.h"				// For various helper functions
#include "stdbool.h"			// For bool datatype

// ------------------------------------------------------------ MACROS

// NOTE: I tried to pick hex-values that are memorable for the purpose
// The highest 4 bits represent the category, the lower 4 bits represent the state
// Module macros
#define WIFIBLE_MODULE_STACK_SIZE			2048
#define WIFIBLE_MODULE_NAME					"WIFIBLE"
#define WIFIBLE_MODULE_INIT_NAME			"WifiBleInit"
#define WIFIBLE_RETVAL						uint8_t
#define WIFIBLE_CMD_DELAY					100U
#define WIFIBLE_UART_BUFFER_SIZE			4096
// Power macros (0xEX)
#define WIFIBLE_POWER_ON					0xE1
#define WIFIBLE_POWER_OFF					0xE0
// Module Config macros (0xCX)
#define WIFIBLE_WIRELESS_MODE				0xCA
#define WIFIBLE_BLUETOOTH_MODE				0xCB
// Status macros (0x0X)
#define WIFIBLE_OK							0x00
#define WIFIBLE_FAILED_COPY					0x01
#define WIFIBLE_FAILED_SEMAPHORE			0x02
#define WIFIBLE_FAILED_FLAG					0x04
#define WIFIBLE_RESERVED					0x08
#define WIFIBLE_ALL_STATUS					0x0F
// Error macros (0xFX)
#define WIFIBLE_NOT_IMPLEMENTED 			0xFA
#define WIFIBLE_MEM_ERROR					0xFB
#define WIFIBLE_POWER_ERROR					0xFC
#define WIFIBLE_RW_ERROR					0xFD
#define WIFIBLE_RESOURCE_ERROR				0xFE
#define WIFIBLE_INIT_ERROR					0xFF

// ------------------------------------------------------------ TYPES

/**
 * @brief Enum to represent Wi-Fi mode
 * @note Not to be confused with module mode, which is either BLUETOOTH or WIFI
 */
enum wifi_mode {
	Station,
	SoftAP,
	Mixed
};

// Module type definitions
/**
 * @brief A structure to provide flags for behaviour change of the module
 * @note Initializes module either as WIFI or BLUETOOTh. Currently only WIFI is implemented.
 */
typedef struct wifible_module {
	uint8_t mode;
} wifible_module_t;
/**
 * @brief Structure to represent Wi-Fi configuration details
 */
typedef struct wifible_wifi_cfg {
	char *ssid;
	char *pwd;
	char *local_ip;
	char *local_gw;
	char *local_netmask;
	char *hostname;
	char *domain;
	uint8_t mode;
} wifible_wifi_cfg_t;
/**
 * @brief Structure to represent Bluetooth configuration details
 */
typedef struct wifible_bt_cfg {
	// TODO define module config data (Bluetooth Mode)
} wifible_bt_cfg_t;

// Not used:
//typedef uint8_t wifible_error_t;

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
/**
 * @brief An initialization task for this module
 *
 * @param argument ... provide wifible_module_t for module behaviour change
 */
void StartWifiBleModule(void *argument);
/**
 * @brief Task for handling Wi-Fi reception
 *
 * @param argument ... ignored / not used
 */
void WifiReceiver(void *argument);
/**
 * @brief Task for handling Bluetooth beacon
 *
 * @param argument ... ignored / not used
 */
void BluetoothBeacon(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
/**
 * @brief Provides a default Wi-Fi configuration
 *
 * @return a pointer to an allocated @ref wifible_wifi_cfg_t structure, which will be filled with default settings
 */
wifible_wifi_cfg_t* wifible_wifi_default_cfg();

/**
 * @brief Clears the provided Wi-Fi configuration
 *
 * @param cfg ... pointer to a @ref wifible_wifi_cfg_t to be free'd from memory
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_wifi_clear_cfg(wifible_wifi_cfg_t *cfg);

/**
 * @brief Initializes the wifi module, using the provided configuration
 *
 * @param cfg ... pointer to a @ref wifible_wifi_cfg_t to be used for configuration
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_wifi_initialization(wifible_wifi_cfg_t *cfg);

/**
 * @brief Provides a default Bluetooth configuration
 *
 * @return a pointer to an allocated @ref wifible_bt_cfg_t structure, which will be filled with default settings
 */
wifible_bt_cfg_t* wifible_bt_default_cfg();

/**
 * @brief Clears the provided Bluetooth configuration
 *
 * @param cfg ... pointer to a @ref wifible_bt_cfg_t to be free'd from memory
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_bt_clear_cfg(wifible_bt_cfg_t *cfg);

/**
 * @brief Initializes the bluetooth module, using the provided configuration
 *
 * @param cfg ... pointer to a @ref wifible_bt_cfg_t to be used for configuration
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_bt_initialization(wifible_bt_cfg_t *cfg);

/**
 * @brief Powers the module on
 * @note Currently not implemented
 *
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_power_module(uint8_t power_state);

/**
 * @brief On IDLE_EVENT signal, checks @ref uart1Buffer and decides copy-mechanism
 *
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_handle_newData(size_t old_pos, size_t new_pos);

/**
 * @brief enters the memory copy function and also clears previously allocated memory
 *
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_enter_copy();

/**
 * @brief Copies data from @ref uart1Buffer in 1 full step
 *
 * @param start_pos ... the start position of the data to copy
 * @param length ... the length from start_pos to end of data
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_attempt_fullCopy(size_t start_pos, size_t length);

/**
 * @brief Copies data from @ref uart1Buffer in 2 partial steps
 * @note 1st partial: start_pos to end of buffer using len1, 2nd partial: start of buffer to len2
 *
 * @param start_pos ... the start position of the data to copy
 * @param len1 ... the length from start to end of buffer
 * @param len2 ... the length from start of buffer to end of data
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_attempt_partialCopy(size_t start_pos, size_t len1, size_t len2);

/**
 * @brief On USER_INPUT signal, forwards global '@ref input' variable to WIFIBLE over USART1
 *
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_handle_userInput();

/**
 * @brief On NEW_REQUEST signal, responds to client via @ref wifible_serve_webPage
 *
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_handle_newRequest();

/**
 * @brief Serves a webpage or a JSON update to a client upon request over USART1
 * @note ALL connections will be dropped in this function, because programming is hard
 *
 * @param link_id ... connection link id of client
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_serve_webPage(uint8_t link_id);

/**
 * @brief Sends the provided buffer to the WIFIBLE module
 * @note This is a suitable callback function for the wrappers in @ref ATcommands.c
 *
 * @param command ... buffer to be sent over USART1
 * @param length ... length of buffer
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
void wifible_send_command(char *command, int length);

/**
 * @brief Prepares peripherals used in this module. Module fails to function without it.
 *
 * @return either error or status macros, @ref WIFIBLE_OK on success
 */
WIFIBLE_RETVAL wifible_prep_peripherals();

/**
 * @brief Handles errors from wifible module functions. Serves as an error-handling wrapper for conditions
 *
 * @param value ... return value of any WIFIBLE module function
 * @return true on failure values, false on successful values
 */
bool wifible_failureHandler(WIFIBLE_RETVAL value);

#endif /* INC_WIFI_BLE_H_ */
