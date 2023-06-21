/*
 * parser.c
 *
 *  Created on: Jun 19, 2023
 *      Author: rini
 */

#include "parser.h"

// ------------------------------------------------------------ STATIC variables
// (configured in application.c)
extern const char *newLine;
extern osThreadId_t parserThreadId;
extern const osThreadAttr_t parser_attr;
extern osThreadId_t wifibleThreadId;
extern osThreadId_t consoleThreadId;
extern osMessageQueueId_t clientPipe;
extern osMessageQueueId_t requestPipe;
extern osMessageQueueId_t responsePipe;
extern osSemaphoreId_t inputSem;
extern char *input;
extern osSemaphoreId_t msgSem;
extern char *newMsg;

// ------------------------------------------------------------ Interrupt routines

// ------------------------------------------------------------ TASKS
void StartParser(void *argument) {
	parser_module_t *module = (parser_module_t*)argument;
	parserThreadId = osThreadNew(Parser, module, &parser_attr);
	osThreadExit();
}

void Parser(void *argument) {
	parser_module_t module = *(parser_module_t*)argument;
	uint32_t flags = 0;
	PARSER_RETVAL ret;
	while(1) {
		flags = osThreadFlagsWait(NEW_INPUT|MSG_EVENT|COPY_REATTEMPT, osFlagsWaitAny, osWaitForever);
		if(flagErrorHandler(flags)) {
			// On flag error, continue loop from beginning
			continue;
		}
		if(flags & NEW_INPUT) {
			ret = parser_handle_input();
			if(parser_failureHandler(ret)) {
				printf_("PARSER: error parsing input: 0x%02x%s", ret, newLine);
			}
		}
		if(flags & MSG_EVENT) {
			ret = parser_handle_message();
			if(parser_failureHandler(ret)) {
				printf_("PARSER: error parsing message: 0x%02x%s", ret, newLine);
			}
		}
		if(flags & COPY_REATTEMPT) {
			ret = parser_handle_copyReattempt();
			if(parser_failureHandler(ret)) {
				printf_("PARSER: error handling copy re-attempt: 0x%02x%s", ret, newLine);
			}
		}
	}
}

// ------------------------------------------------------------ PRIVATE

PARSER_RETVAL parser_handle_input() {
	osStatus_t ret;
	uint32_t flag_return;
	PARSER_RETVAL parse_return;
	// Request semaphore to lock input
	ret = osSemaphoreAcquire(inputSem, MODULE_SEM_TIMEOUT);
	if(osStatusHandler(ret)) {
		return PARSER_SEMAPHORE_ERROR;
	}
	// Parse input
	parse_return = parser_parseInput();
	// Buffer is free'd by wifible
	// Release semaphore to free input
	ret = osSemaphoreRelease(inputSem);
	if(osStatusHandler(ret)) {
		return PARSER_SEMAPHORE_ERROR;
	}
	// Check parsing for data errors
	if(parser_failureHandler(parse_return)) {
		return PARSER_DATA_ERROR;
	}
	// Notify wifible about user input
	flag_return = osThreadFlagsSet(wifibleThreadId, USER_INPUT);
	if(flagErrorHandler(flag_return)) {
		return PARSER_FAILED;
	}
	return PARSER_OK;
}

PARSER_RETVAL parser_handle_message() {
	osStatus_t ret;
	PARSER_RETVAL parse_return;
	// Request semaphore to lock input
	ret = osSemaphoreAcquire(msgSem, MODULE_SEM_TIMEOUT);
	if(osStatusHandler(ret)) {
		return PARSER_SEMAPHORE_ERROR;
	}
	// Parse the newMsg
	parse_return = parser_parseMessage();
	// Free allocated buffer
	vPortFree(input);
	newMsg = NULL;
	// Release semaphore to free input
	ret = osSemaphoreRelease(msgSem);
	if(osStatusHandler(ret)) {
		return PARSER_SEMAPHORE_ERROR;
	}
	// Check parsing for data errors
	if(parser_failureHandler(parse_return)) {
		return PARSER_DATA_ERROR;
	}
	return PARSER_OK;
}
PARSER_RETVAL parser_handle_copyReattempt() {
	uint32_t flag_return = osThreadFlagsSet(wifibleThreadId, COPY_REATTEMPT);
	if(flagErrorHandler(flag_return)) {
		return PARSER_FAILED;
	}
	return PARSER_OK;
}

PARSER_RETVAL parser_parseInput() {
	// TODO Check user input for errors in parseInput
	// Print the input (also clears console echo)
	printf_("\rPARSER: input: %s%s", input, newLine);
	return PARSER_OK;
}

PARSER_RETVAL parser_parseMessage() {
	if(newMsg[0] != '+' || !(newMsg[0] >= '0' || newMsg[0] <= '9') ) {
		return PARSER_OK;
	}

	// temporary implementation
	char* line = strtok(newMsg, "\n"); // Tokenize the message string into lines

	while (line != NULL) {
		if (strncmp(line, "+STA_CONNECTED:", 15) == 0) {
			// This line contains the MAC address of the connected station
			char* mac_address = line + 15;
			printf_("MAC address: %s\n", mac_address);
		} else if (strncmp(line, "+DIST_STA_IP:", 13) == 0) {
			// This line contains the MAC and IP address of the connected station
			char* mac_address = strtok(line + 13, ",");
			char* ip_address = strtok(NULL, ",");
			printf_("MAC address: %s, IP address: %s\n", mac_address, ip_address);
		} else if (strncmp(line, "+IPD,", 5) == 0) {
			// This line contains the HTTP request
			char* request = strchr(line, ':') + 1;
			printf_("HTTP request: %s\n", request);
		} else if (line[0] >= '0' || line[0] <= '9') {
			// This line indicates a connect or disconnect event
			if (strstr(line, "CONNECT") != NULL) {
				printf_("Connect event\n");
			} else if (strstr(line, "DISCONNECT") != NULL) {
				printf_("Disconnect event\n");
			}
		} else {
			// This line contains an HTTP header
			char* header_name = strtok(line, ":");
			char* header_value = strtok(NULL, ":");
			printf_("HTTP header: %s = %s\n", header_name, header_value);
		}

		line = strtok(NULL, "\n"); // Get the next line
	}


	// Check message type
	// Parse message into struct
	// Send data to pipe
	// Notify webserver about data
	return PARSER_OK;
}

bool parser_failureHandler(PARSER_RETVAL value) {
	switch(value) {
	case PARSER_OK:
		return false;
	case PARSER_FAILED:
		printf_("PARSER: Failure!%s", newLine);
		break;
	case PARSER_MEM_ERROR:
		printf_("PARSER: Out of Memory!%s", newLine);
		break;
	case PARSER_RESERVED_ERROR:
		printf_("PARSER: Reserved error!%s", newLine);
		break;
	case PARSER_DATA_ERROR:
		printf_("PARSER: Data error!%s", newLine);
		break;
	case PARSER_SEMAPHORE_ERROR:
		printf_("PARSER: Semaphore error!%s", newLine);
		break;
	case PARSER_INIT_ERROR:
		printf_("PARSER: Initialization error!%s", newLine);
		break;
	}
	return true;
}
