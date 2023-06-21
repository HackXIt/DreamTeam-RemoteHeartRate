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
#define WIFIBLE_MODULE_STACK_SIZE			512
#define WIFIBLE_MODULE_NAME					"WIFIBLE"
#define WIFIBLE_MODULE_INIT_NAME			"WifiBleInit"
#define WIFIBLE_RETVAL						uint8_t
#define WIFIBLE_CMD_DELAY					1000U
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
#define WIFIBLE_RESOURCE_ERROR				0xFC
#define WIFIBLE_RW_ERROR					0xFD
#define WIFIBLE_POWER_ERROR					0xFE
#define WIFIBLE_INIT_ERROR					0xFF

// ------------------------------------------------------------ TYPES

enum wifi_mode {
	Station,
	SoftAP,
	Mixed
};

// Module type definitions
typedef struct wifible_module {
	uint8_t mode;
} wifible_module_t;
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
typedef struct wifible_bt_cfg {
	// TODO define module config data (Bluetooth Mode)
} wifible_bt_cfg_t;

// Not used:
//typedef uint8_t wifible_error_t;

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
void StartWifiBleModule(void *argument);
void WifiReceiver(void *argument);
void BluetoothBeacon(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
wifible_wifi_cfg_t* wifible_wifi_default_cfg();
WIFIBLE_RETVAL wifible_wifi_clear_cfg(wifible_wifi_cfg_t *cfg);
WIFIBLE_RETVAL wifible_wifi_initialization(wifible_wifi_cfg_t *cfg);
wifible_bt_cfg_t* wifible_bt_default_cfg();
WIFIBLE_RETVAL wifible_bt_clear_cfg(wifible_bt_cfg_t *cfg);
WIFIBLE_RETVAL wifible_bt_initialization(wifible_bt_cfg_t *cfg);
WIFIBLE_RETVAL wifible_power_module(uint8_t power_state);
WIFIBLE_RETVAL wifible_handle_newData(size_t old_pos, size_t new_pos);
WIFIBLE_RETVAL wifible_attempt_fullCopy(size_t start_pos, size_t length);
WIFIBLE_RETVAL wifible_attempt_partialCopy(size_t start_pos, size_t len1, size_t len2);
WIFIBLE_RETVAL wifible_handle_userInput();
void wifible_send_command(char *command, int length);
WIFIBLE_RETVAL wifible_prep_peripherals();
void wifible_serve_webPage();
bool wifible_failureHandler(WIFIBLE_RETVAL value);

#endif /* INC_WIFI_BLE_H_ */
