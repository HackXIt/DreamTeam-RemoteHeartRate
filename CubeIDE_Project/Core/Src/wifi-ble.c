/*
 * wifi-ble.c
 *
 *  Created on: May 22, 2023
 *      Author: rini
 */

#include "wifi-ble.h"

// ------------------------------------------------------------ TASKS

void WiFiInitialization(void *argument) {

}

void BluetoothInitialization(void *argument) {

}

void StartTcpServer(void *argument) {

}

void StartBluetoothBeacon(void *argument) {

}

void StartWifiBleModule(void *argument) {
	// parse argument
	// depending on argument, either initialize WiFi or Bluetooth
	// depending on argument, start the server or beacon
}

// ------------------------------------------------------------ PRIVATE

wifible_cfg_t* wifible_default_cfg() {
	// TODO Implement WiFi-BLE default configuration
	return NULL;
}

void wifible_setup(wifible_cfg_t *cfg) {
	// TODO Implement WiFi-BLE Setup function
}

WIFIBLE_RETVAL wifible_init(wifible_t *ctx, wifible_cfg_t *cfg) {
	// TODO Implement WiFi-BLE Init function
	return WIFIBLE_NOT_IMPLEMENTED;
}

WIFIBLE_RETVAL wifible_power_module(wifible_t *ctx, uint8_t power_state) {
	// TODO Implement WiFi-BLE Power function
	return WIFIBLE_NOT_IMPLEMENTED;
}

//void wifible_send(wifible_t *ctx, char *data_buf, uint16_t data_len);
void wifible_generic_write(wifible_t *ctx, char *data_buf, uint16_t data_len) {
	// TODO Implement WiFi-BLE write function
}

//uint16_t wifible_receive(wifible_t *ctx, char *data_buf, uint16_t max_len);
uint16_t wifible_generic_read(wifible_t *ctx, char *data_buf, uint16_t max_len) {
	// TODO Implement WiFi-BLE read function
	return 0;
}

WIFIBLE_RETVAL wifible_send_command(wifible_t *ctx, char *command) {
	// TODO Implement WiFi-BLE command function
	return WIFIBLE_NOT_IMPLEMENTED;
}
