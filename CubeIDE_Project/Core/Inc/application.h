/*
 * moduleFlags.h
 *
 *  Created on: May 30, 2023
 *      Author: rini
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#define MODULE_INIT_SIZE		256
#define MODULE_SEM_TIMEOUT		1000U
#define MESSAGE_SEMAPHORE		"Message"
#define MESSAGE_MAX_COUNT		1U
#define MESSAGE_INIT_COUNT		1U
#define INPUT_SEMAPHORE			"Input"
#define INPUT_MAX_COUNT			1U
#define INPUT_INIT_COUNT		1U

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
// ------------------------------------------------------------ parser event flags
#define MSG_EVENT		(1 << 8)
#define NEW_INPUT		(1 << 9)
// ------------------------------------------------------------ web server event flags
#define NEW_CONNECTION	(1 << 12)
#define NEW_REQUEST		(1 << 13)
// ------------------------------------------------------------ application wide flags
#define IDLE_EVENT		(1 << 16)
#define COPY_REATTEMPT	(1 << 17)

#endif /* INC_APPLICATION_H_ */
