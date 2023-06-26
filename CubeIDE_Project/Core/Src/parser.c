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
extern osThreadId_t webserverThreadId;
/*
extern osMessageQueueId_t clientPipe;
extern osMessageQueueId_t requestPipe;
extern osMessageQueueId_t responsePipe;
*/
extern osSemaphoreId_t inputSem;
extern char *input;
extern osSemaphoreId_t msgSem;
extern char *newMsg;
extern uint8_t responseLink;
extern osSemaphoreId_t httpSem;
extern char *http_data;
extern uint8_t clients[4];
extern uint8_t requestType;
#ifdef USE_WEBSERVER
extern char request_buffer[PARSER_REQUEST_BUFFER_SIZE];
extern char response_buffer[PARSER_RESPONSE_BUFFER_SIZE];
#endif

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
	uint32_t flag_return;
	PARSER_RETVAL parser_return;
	// Request semaphore to lock input
	if((parser_return = acquire_semaphore(inputSem, MODULE_SEM_TIMEOUT)) != PARSER_OK) {
		return parser_return;
	}
	// -------------------------------- START OF PARSE
	// Parse input
	if((parser_return = parser_parseInput()) != PARSER_OK) {
		return parser_return;
	}
	// -------------------------------- END OF PARSE
	if((parser_return = release_semaphore(inputSem)) != PARSER_OK) {
		return parser_return;
	}
	// NOTE: input Buffer is free'd by CONSOLE module
	// Notify wifible about user input
	flag_return = osThreadFlagsSet(wifibleThreadId, USER_INPUT);
	if(flagErrorHandler(flag_return)) {
		return PARSER_FAILED;
	}
	return PARSER_OK;
}

PARSER_RETVAL parser_handle_message() {
	PARSER_RETVAL parser_return;
	// Request semaphore to lock input
	if((parser_return = acquire_semaphore(msgSem, MODULE_SEM_TIMEOUT)) != PARSER_OK) {
		return parser_return;
	}
	// -------------------------------- START OF PARSE
	// Parse message (this function also releases the semaphore)
	parser_return = parser_parseMessage();
	if(parser_failureHandler(parser_return)) {
		// On any parsing error
		return PARSER_DATA_ERROR;
	}
	// -------------------------------- END OF PARSE
	// NOTE: newMsg Buffer is free'd by WIFIBLE module
	// TODO Notify webserver about network data (if any)
	if(parser_return & PARSER_NEW_REQUEST) {
#ifndef USE_WEBSERVER
		uint32_t flag_return = osThreadFlagsSet(wifibleThreadId, NEW_RESPONSE);
#endif
#ifdef USE_WEBSERVER
		uint32_t flag_return = osThreadFlagsSet(webserverThreadId, NEW_REQUEST);
#endif
		if(flagErrorHandler(flag_return)) {
			return PARSER_FAILED;
		}
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
	PARSER_RETVAL parser_return = PARSER_OK;
	// Ignore configuration messages
	if(newMsg[0] != '+' && !(newMsg[0] >= '0' && newMsg[0] <= '9') ) {
		// Free newMsg to other threads
		release_semaphore(msgSem);
		return parser_return;
	}

	// Make local copy of message
	uint16_t msg_length = strlen(newMsg);
	char *msg = pvPortMalloc(sizeof(char)*msg_length+1);
	memcpy(msg, newMsg, msg_length);
	msg[msg_length] = '\0'; // Null terminate copied string

	// Free newMsg to other threads
	release_semaphore(msgSem);

	// temporary implementation
	char *outer_saveptr = NULL;
	char *inner_saveptr = NULL;
	char *line = strtok_r(msg, newLine, &outer_saveptr); // Tokenize the message string into lines

	// TODO Change all MAGIC Numbers to readable & informative values

	while (line != NULL) {
		if (strncmp(line, "+STA_CONNECTED:", 15) == 0) {
			// This line contains the MAC address of a newly connected station
			// Example: +STA_CONNECTED:"c4:23:60:66:ba:8e"
			char *identifier = strtok_r(line, ":", &inner_saveptr);
			char *mac_address = strtok_r(NULL, "\"", &inner_saveptr);
			printf_("PARSER: (%s)%sNew WLAN connection: %s%s", identifier, newLine, mac_address, newLine);
		} else if (strncmp(line, "+STA_DISCONNECTED:", 18) == 0) {
			// This line contains the MAC address of a disconnected station
			// Example: +STA_DISCONNECTED:"c4:23:60:66:ba:8e"
			char *identifier = strtok_r(line, ":", &inner_saveptr);
			char *mac_address = strtok_r(NULL, "\"", &inner_saveptr);
			printf_("PARSER: (%s)%sDropped WLAN connection: %s%s", identifier, newLine, mac_address, newLine);
		} else if (strncmp(line, "+DIST_STA_IP:", 13) == 0) {
			// This line contains the IP address of an already connected station
			// Example: +DIST_STA_IP:"c4:23:60:66:ba:8e","192.1.0.2"
			// NOTE: somehow the 1st quote gets lost, but I don't need it anyways.
			char *identifier = strtok_r(line, ":", &inner_saveptr);
			char *mac_address = strtok_r(NULL, "\"", &inner_saveptr);
			strtok_r(NULL, "\"", &inner_saveptr); // Drop 3rd quote
			char *ip_address = strtok_r(NULL, "\"", &inner_saveptr);
			printf_("PARSER: (%s)%sDevice with MAC address %s got IP address: %s%s", identifier, newLine, mac_address, ip_address, newLine);
		} else if (strncmp(line, "+IPD", 4) == 0) {
			// This line contains network data
			// Example: +IPD,0,73:GET / HTTP/1.1
			// ... data ...
			char *identifier = strtok_r(line, ",", &inner_saveptr);
			char *link_id = strtok_r(NULL, ",", &inner_saveptr);
			// NOTE: It is hard to verify if this length is correct
			// I am going to assume it's correct, at worst, I have a few bytes too much in my buffer
			// Also, it is very likely that strtok_r fucked with the data.
			char *data_length = strtok_r(NULL, ":", &inner_saveptr);
			// Send request to webserver to parse & handle
			char *request = strtok_r(NULL, "", &inner_saveptr);
#ifdef USE_WEBSERVER
			uint16_t request_length = strlen(request);
			char *data = strtok_r(NULL, "", &outer_saveptr);
			data = data+1; // Skip \n since strtok kept it
			uint16_t reqData_length = strlen(data);
			uint8_t newLine_length = strlen(newLine);
			uint16_t total_length = request_length + newLine_length + reqData_length;
			if(acquire_semaphore(httpSem, MODULE_SEM_TIMEOUT) == APPLICATION_OK) {
				strncpy(request_buffer, request, request_length);
				strncpy(request_buffer+request_length, newLine, newLine_length);
				strncpy(request_buffer+newLine_length+request_length, data, reqData_length);
				// TODO could check written against data_length here
				release_semaphore(httpSem); // FIXME Assuming success
				parser_return = PARSER_NEW_REQUEST;
				break;
			}
#endif
			/*
			char *request_type = strtok_r(NULL, " ", &inner_saveptr);
			strtok_r(NULL, " ", &inner_saveptr); // Drop slash with space
			char *protocol = strtok_r(NULL, "/", &inner_saveptr);
			char *protocol_version = strtok_r(NULL, "", &inner_saveptr);
			char *upgrade = strtok_r(NULL, "Upgrade:", &outer_saveptr);
			char *key;
			if(upgrade != NULL) {
				printf_("TMP: %s", strtok_r(NULL, "-Key:", &outer_saveptr));
				key = strtok_r(NULL, newLine, &outer_saveptr);
				printf_("Key: %s", key);
			}
			printf_("PARSER: (%s)%sNetwork data from Link %s with length %s%sRequest_Type: %s%sProtocol: %s (Version: %s)%s",
					identifier, newLine, link_id, data_length, newLine, request_type, newLine, protocol, protocol_version, newLine);
			*/
#ifdef PARSER_DEBUG
			printf_("PARSER: (%s) Network data from Link %s with length %s:%s%s%s%s",
								identifier, link_id, data_length, newLine, request, data, newLine);
#endif
#ifndef PARSER_DEBUG
			printf_("PARSER: (%s) Network data from Link %s with length %s: %s%s",
											identifier, link_id, data_length, request, newLine);
#endif
			char *accept_header = NULL;
			do {
				line = strtok_r(NULL, newLine, &outer_saveptr);
				if(strncmp(line, "Accept: ", 8) == 0) {
					accept_header = line+8;
					break;
				}
			} while(accept_header == NULL && line != NULL);
#ifndef	USE_WEBSERVER
			if(sscanf(link_id, "%hhu", &responseLink) != 1) {
				parser_return = PARSER_DATA_ERROR;
			} else {
				if(strncmp(accept_header, "application/json", 16) == 0) {
					requestType = JSON_REQUEST;
				} else {
					requestType = HTML_REQUEST;
				}
				parser_return = PARSER_NEW_REQUEST;
			}
#endif
			break;
		} else if (line[0] >= '0' && line[0] <= '9') {
			// A line starting with a number indicates a connection event from a specific link
			// Example 1: 0,CONNECT
			// Example 2: 0,CONNECT FAIL
			// Example 3: 0,CLOSED
			char *link_id = strtok_r(line, ",", &inner_saveptr);
			char *event = strtok_r(NULL, ",", &inner_saveptr);
			sscanf(link_id, "%hhu", &responseLink);
			if(strcmp(event, "CONNECT") == 0) {
				clients[responseLink] = CLIENT_CONNECTED;
			} else if(strcmp(event, "CONNECT FAIL") == 0) {
				clients[responseLink] = CLIENT_FAILED;
			} else if(strcmp(event, "CLOSED") == 0) {
				clients[responseLink] = CLIENT_CLOSED;
			}
			printf_("PARSER: Link %s => %s event%s", link_id, event, newLine);
		} else {
			printf_("PARSER: Unhandled line: %s%s", line, newLine);
		}
		line = strtok_r(NULL, newLine, &outer_saveptr); // Get the next line
	}
	vPortFree(msg); // Clear copied data
	// TODO Check message type for valid request
	// TODO Parse retrieved strings into corresponding structs
	// TODO Send struct over pipe to webserver
	// TODO Notify webserver about new data
	return parser_return;
}

bool parser_failureHandler(PARSER_RETVAL value) {
	switch(value) {
	case PARSER_OK:
		return false;
	case PARSER_NEW_REQUEST:
		return false;
	case PARSER_FAILED:
		printf_("PARSER: Failure!%s", newLine);
		break;
	case PARSER_FAILED_SEMAPHORE:
		printf_("PARSER: Failure in semaphore!%s", newLine);
	case PARSER_MEM_ERROR:
		printf_("PARSER: Out of Memory!%s", newLine);
		break;
	case PARSER_RESERVED_ERROR:
		printf_("PARSER: Reserved error!%s", newLine);
		break;
	case PARSER_DATA_ERROR:
		printf_("PARSER: Data error!%s", newLine);
		break;
	case PARSER_RESOURCE_ERROR:
		printf_("PARSER: Resource error!%s", newLine);
		break;
	case PARSER_INIT_ERROR:
		printf_("PARSER: Initialization error!%s", newLine);
		break;
	}
	return true;
}
