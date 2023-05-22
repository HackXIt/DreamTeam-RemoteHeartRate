/*
 * wifi-ble.h
 *
 *  Created on: May 22, 2023
 *      Author: rini
 */

#ifndef INC_WIFI_BLE_H_
#define INC_WIFI_BLE_H_

// ------------------------------------------------------------ IMPORTS
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "usart.h"

// ------------------------------------------------------------ MACROS

// Module macros
#define WIFIBLE_RETVAL		uint8_t

// Status macros
#define WIFIBLE_OK
// Error macros
#define WIFIBLE_NOT_IMPLEMENTED 0xFA
#define WIFIBLE_WRITE_ERROR		0xFC
#define WIFIBLE_READ_ERROR		0xFD
#define WIFIBLE_POWER_ERROR		0xFE
#define WIFIBLE_INIT_ERROR		0xFF

// ------------------------------------------------------------ TYPES

// Module type definitions
typedef struct wifible {
	// TODO define module object data
} wifible_t;

typedef struct wifible_cfg {
	// TODO define module config data
} wifible_cfg_t;

typedef uint8_t wifible_error_t;

// ------------------------------------------------------------ STATIC VARIABLES
// To-be-defined if any...

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
void WiFiInitialization(void *argument);
void BluetoothInitialization(void *argument);
void StartTcpServer(void *argument);
void StartBluetoothBeacon(void *argument);
void StartWifiBleModule(void *argument);

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
wifible_cfg_t* wifible_default_cfg();
void wifible_setup(wifible_cfg_t *cfg);
WIFIBLE_RETVAL wifible_init(wifible_t *ctx, wifible_cfg_t *cfg);
WIFIBLE_RETVAL wifible_power_module(wifible_t *ctx, uint8_t power_state);
//void wifible_send(wifible_t *ctx, char *data_buf, uint16_t data_len);
void wifible_generic_write(wifible_t *ctx, char *data_buf, uint16_t data_len);
//uint16_t wifible_receive(wifible_t *ctx, char *data_buf, uint16_t max_len);
uint16_t wifible_generic_read(wifible_t *ctx, char *data_buf, uint16_t max_len);
WIFIBLE_RETVAL wifible_send_command(wifible_t *ctx, char *command);


#endif /* INC_WIFI_BLE_H_ */
