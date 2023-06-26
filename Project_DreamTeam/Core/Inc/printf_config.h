/*
 * printf_config.h
 *
 *  Created on: Jun 18, 2023
 *      Author: rini
 */

#ifndef INC_PRINTF_CONFIG_H_
#define INC_PRINTF_CONFIG_H_

/* NOTE: Environment variable PRINTF_INCLUDE_CONFIG_H must be set to 1
 * This header file controls certain features of the used printf library.
 * Without the environment variable, these definitions will be ignored.
 */

#define SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS		0
#define PRINTF_SUPPORT_LONG_LONG					0
#define PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS		0
#define PRINTF_SUPPORT_WRITEBACK_SPECIFIER			0

/* NOTE: Definition of putchar_(char c) is inside usart.c
 */

#endif /* INC_PRINTF_CONFIG_H_ */
