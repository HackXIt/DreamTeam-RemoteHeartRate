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

bool flagErrorHandler(uint32_t flag);
bool threadErrorHandler(osThreadId_t id, char *msg);
bool halStatusHandler(HAL_StatusTypeDef val);
bool osStatusHandler(osStatus_t val);
void translate_I2C_Error(I2C_HandleTypeDef *hi2c);
void translate_UART_Error(UART_HandleTypeDef *huart);
void printDebug(const char *msg);

#endif /* INC_HELPER_H_ */
