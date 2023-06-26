/*
 * helper.h
 *
 *  Created on: Jun 19, 2023
 *      Author: rini
 */

#ifndef INC_HELPER_H_
#define INC_HELPER_H_

#include "stm32l4xx_hal.h"
#include "stdbool.h"
#include "cmsis_os2.h"
#include "printf.h"
// LowLevel drivers
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_i2c.h"

/**
 * @brief checks provided flag for osFlagsErrors
 * @note This is only to be used for flag functions in CMSIS
 *
 * @param flag ... return value of osFlag function
 * @return true on failure, false on success
 */
bool flagErrorHandler(uint32_t flag);

/**
 * @brief checks provided thread id for null
 *
 * @param id ... osThreadId of created thread
 * @param msg ... name of thread or any other arbitrary message
 * @return true on failure, false on success
 */
bool threadErrorHandler(osThreadId_t id, char *msg);

/**
 * @brief checks the provided HAL status for errors
 * @note This is typically used after a HAL operation to ensure its success
 *
 * @param val ... return value of a HAL operation
 * @return true on failure, false on success
 */
bool halStatusHandler(HAL_StatusTypeDef val);

/**
 * @brief checks the provided OS status for errors
 * @note This is typically used after an os operation to ensure its success
 *
 * @param val ... return value of an os operation
 * @return true on failure, false on success
 */
bool osStatusHandler(osStatus_t val);

/**
 * @brief translates and outputs the error message for I2C errors
 * @note This is typically used for debugging purposes
 *
 * @param hi2c ... pointer to a @ref I2C_HandleTypeDef structure that contains
 *                the configuration information for I2C module.
 */
void translate_I2C_Error(I2C_HandleTypeDef *hi2c);

/**
 * @brief translates and outputs the error message for UART errors
 * @note This is typically used for debugging purposes
 *
 * @param huart ... pointer to a @ref UART_HandleTypeDef structure that contains
 *                  the configuration information for UART module.
 */
void translate_UART_Error(UART_HandleTypeDef *huart);

/**
 * @brief outputs a debug message (used for debugging purposes)
 * @note If DEBUG is not defined, function content will not be compiled and no output occurs
 *
 * @param msg ... arbitrary message string to output
 */
void printDebug(const char *msg);

#endif /* INC_HELPER_H_ */
