/*
 * moduleFlags.h
 *
 *  Created on: May 30, 2023
 *      Author: rini
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

// ------------------------------------------------------------ IMPORTS
#include "cmsis_os2.h"			// For CMSIS API functions

// Temporary macros for debugging
/* NOTE: WIFIBLE_SKIP_CONFIGURATION
 * Skips the configuration process on WIFIBLE.
 * There is no need to configure after 1st configuration, WIFIBLE keeps config inbetween flashes.
 * WIFIBLE needs to be re-configured whenever power to WIFIBLE goes out.
 */
//#define WIFIBLE_SKIP_CONFIGURATION

// I couldn't get the websocket server to work, because it just was too complicated
// I also failed to find a suitable library, so all webserver code is ignored.
//#define USE_WEBSERVER

// ------------------------------------------------------------ Application wide macros
// Application macros
#define MODULE_INIT_SIZE		256
#define MODULE_SEM_TIMEOUT		1000U
#define MAX_PAGE_SIZE			2048U
#define MAX_UPDATE_SIZE			128U
#define MESSAGE_SEMAPHORE		"Message"
#define MESSAGE_MAX_COUNT		1U
#define MESSAGE_INIT_COUNT		1U
#define INPUT_SEMAPHORE			"Input"
#define INPUT_MAX_COUNT			1U
#define INPUT_INIT_COUNT		1U
#define HTTP_SEMAPHORE			"HTTP"
#define HTTP_MAX_COUNT			1U
#define HTTP_INIT_COUNT			1U
#define I2C_SEMAPHORE			"I2C"
#define I2C_INIT_COUNT			0U
#define HEARTRATE_RAM_SIZE		4
#define APPLICATION_RETVAL		uint8_t
// Status macros (0x0X)
#define APPLICATION_OK			0x00
#define SEMAPHORE_FAILURE		0x02
// Client macros
#define CLIENT_CLOSED			0x00
#define CLIENT_CONNECTED		0xC1
#define CLIENT_2ND_CONNECT		0xC2
#define CLIENT_FAILED			0xC4
// Request macros
#define HTML_REQUEST			0x00
#define JSON_REQUEST			0xD1

/* NOTE: Event flags in FreeRTOS
 * DON'T USE THESE BITS: 0bXXXXXXXX000000000000000000000000
 * (the highest bits in a 32 bit value, i.e. 24 to 31)
 * They are not used in FreeRTOS and will cause problems.
 */

/* NOTE: The application event flags are seperated by BYTE
 * 1st Half of 1st Byte (0-3)	=> console monitor
 * 2nd Half of 1st Byte (4-7)	=> wifi-ble
 * 1st Half of 2nd Byte (8-11)	=> parser
 * 2nd Half of 2nd Byte (12-15)	=> web server
 * 3rd Byte (16-23)				=> application-wide
 * For readability, the flag numbers should be ordered
 */
// ------------------------------------------------------------ console event flags
#define RX_BYTE			(1 << 0)
#define BUFFER_OVERRUN	(1 << 1)
// ------------------------------------------------------------ wifi-ble event flags
#define USER_INPUT		(1 << 4)
#define NEW_RESPONSE	(1 << 5)
// ------------------------------------------------------------ parser event flags
#define MSG_EVENT		(1 << 8)
#define NEW_INPUT		(1 << 9)
// ------------------------------------------------------------ web server event flags
#define NEW_CONNECTION	(1 << 12)
#define NEW_REQUEST		(1 << 13)
// ------------------------------------------------------------ application wide flags
#define IDLE_EVENT		(1 << 16)
#define COPY_REATTEMPT	(1 << 17)
#define I2C_EVENT		(1 << 18)
#define USART1_READY	(1 << 19)

// ------------------------------------------------------------ Application wide functions
/**
 * @brief Attempts to acquire a semaphore with the given timeout
 *
 * @param semaphore ... id of semaphore to get
 * @param timeout ... will return after specified timeout
 * @return SEMAPHORE_FAILURE on any osFailure and APPLICATION_OK on success
 */
APPLICATION_RETVAL acquire_semaphore(osSemaphoreId_t semaphore, uint32_t timeout);
/**
 * @brief Attempts to release a semaphore
 *
 * @param semaphore ... id of semaphore to release
 * @return SEMAPHORE_FAILURE on any osFailure and APPLICATION_OK on success
 */
APPLICATION_RETVAL release_semaphore(osSemaphoreId_t semaphore);

#endif /* INC_APPLICATION_H_ */
