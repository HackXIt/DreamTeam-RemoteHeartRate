/*
 * helper.c
 *
 *  Created on: Jun 19, 2023
 *      Author: rini
 */

#include "helper.h"

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

void printDebug(const char *msg) {
#ifdef DEBUG
	printf_("DEBUG: %s => ", msg);
#endif
}
