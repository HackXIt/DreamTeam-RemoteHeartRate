/*
 * helper.c
 *
 *  Created on: Jun 19, 2023
 *      Author: rini
 */

#include "helper.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

bool flagErrorHandler(uint32_t flag) {
	if(!(flag & osFlagsError)) {
		// No error indicator == no error
		return false;
	}
	switch(flag) {
	case osFlagsErrorUnknown:
		printDebug("osFlagsErrorUnknown");
		// TODO Handle osFlagsErrorUnknown in flagErrorHandler
		break;
	case osFlagsErrorTimeout:
		printDebug("osFlagsErrorTimeout");
		// TODO Handle osFlagsErrorTimeout in flagErrorHandler
		break;
	case osFlagsErrorResource:
		printDebug("osFlagsErrorResource");
		// TODO Handle osFlagsErrorResource in flagErrorHandler
		break;
	case osFlagsErrorParameter:
		printDebug("osFlagsErrorParameter");
		// TODO Handle osFlagsErrorParameter in flagErrorHandler
		break;
	case osFlagsErrorISR:
		printDebug("osFlagsErrorISR");
		// TODO Handle osFlagsErrorISR in flagErrorHandler
		break;
	}
	return true;
}

bool threadErrorHandler(osThreadId_t id, char *msg) {
	if(id != NULL) {
		return false;
	}
	printf_("Failed to create thread: %s\r\n", msg);
	return true;
}

bool halStatusHandler(HAL_StatusTypeDef val) {
	switch(val) {
	case HAL_OK:
		return false;
	case HAL_ERROR:
		printDebug("HAL_ERROR");
		// TODO Handle HAL_ERROR in halStatusHandler
		break;
	case HAL_BUSY:
		printDebug("HAL_BUSY");
		// TODO Handle HAL_BUSY in halStatusHandler
		break;
	case HAL_TIMEOUT:
		printDebug("HAL_TIMEOUT");
		// TODO Handle HAL_TIMEOUT in halStatusHandler
		break;
	}
	return true;
}

bool osStatusHandler(osStatus_t val) {
	switch(val) {
	case osOK:
		return false;
	case osError:
		printDebug("osError");
		// TODO Handle osError in osStatusHandler
		break;
	case osErrorTimeout:
		printDebug("osErrorTimeout");
		// TODO Handle osErrorTimeout in osStatusHandler
		break;
	case osErrorResource:
		printDebug("osErrorResource");
		// TODO Handle osErrorResource in osStatusHandler
		break;
	case osErrorParameter:
		printDebug("osErrorParameter");
		// TODO Handle osErrorParameter in osStatusHandler
		break;
	case osErrorNoMemory:
		printDebug("osErrorNoMemory");
		// TODO Handle osErrorNoMemory in osStatusHandler
		break;
	case osErrorISR:
		printDebug("osErrorISR");
		// TODO Handle osErrorISR in osStatusHandler
		break;
	case osStatusReserved:
		printDebug("osStatusReserved");
		return false;
	}
	return true;
}

void translate_I2C_Error(I2C_HandleTypeDef *hi2c) {
	uint32_t error_value = HAL_I2C_GetError(hi2c);
	printf_("I2C: ");
	switch(error_value) {
	case HAL_I2C_ERROR_NONE:
		printf_("No error");
		break;
	case HAL_I2C_ERROR_BERR:
		printf_("BERR error");
		LL_I2C_ClearFlag_BERR(hi2c->Instance);
		break;
	case HAL_I2C_ERROR_ARLO:
		printf_("ARLO error");
		LL_I2C_ClearFlag_ARLO(hi2c->Instance);
		break;
	case HAL_I2C_ERROR_AF:
		printf_("ACKF error");
		break;
	case HAL_I2C_ERROR_OVR:
		printf_("OVR error");
		break;
	case HAL_I2C_ERROR_DMA:
		printf_("DMA transfer error");
		break;
	case HAL_I2C_ERROR_TIMEOUT:
		printf_("Timeout error");
		break;
	case HAL_I2C_ERROR_SIZE:
		printf_("Size Management error");
		break;
	case HAL_I2C_ERROR_DMA_PARAM:
		printf_("DMA Parameter error");
		break;
	default:
		printf_("Multiple I2C errors");
		if(error_value & HAL_I2C_ERROR_BERR) LL_I2C_ClearFlag_BERR(hi2c->Instance);
		if(error_value & HAL_I2C_ERROR_ARLO) LL_I2C_ClearFlag_ARLO(hi2c->Instance);
		break;
	}
	printf_(" (0x%08lx)\r\n", error_value);
}

void translate_UART_Error(UART_HandleTypeDef *huart) {
	uint32_t error_value = HAL_UART_GetError(huart);
	printf_("UART: ");
	switch(error_value){
	case HAL_UART_ERROR_NONE:
		printf_("No error");
		break;
	case HAL_UART_ERROR_PE:
		printf_("Parity error");
		break;
	case HAL_UART_ERROR_NE:
		printf_("Noise error");
		break;
	case HAL_UART_ERROR_FE:
		printf_("Frame error");
		break;
	case HAL_UART_ERROR_ORE:
		printf_("Overrun error");
		break;
	case HAL_UART_ERROR_DMA:
		printf_("DMA transfer error");
		break;
	case HAL_UART_ERROR_RTO:
		printf_("Receiver Timeout error");
		break;
	default:
		printf_("Multiple UART errors");
		break;
	}
	printf_(" (0x%08lx)\r\n", error_value);
}

void printDebug(const char *msg) {
#ifdef DEBUG
	printf_("DEBUG: %s => ", msg);
#endif
}
