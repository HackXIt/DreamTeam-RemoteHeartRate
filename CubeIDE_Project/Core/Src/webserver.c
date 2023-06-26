/*
 * webserver.c
 *
 *  Created on: Jun 19, 2023
 *      Author: rini
 */

/* NOTE: Here's a sample of a complete web-request as it is received on UART1
 * +STA_CONNECTED:"02:d2:42:8c:26:44"
 * +DIST_STA_IP:"02:d2:42:8c:26:44","192.1.0.2"
 * 0,CONNECT
 *
 * +IPD,0,322:GET / HTTP/1.1
 * Host: 192.1.0.1
 * User-Agent: Mozilla/5.0 (Android 12; Mobile; rv:109.0) Gecko/114.0 Firefox/114.0
 * Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*\/*;q=0.8
 * Accept-Language: de-AT
 * Accept-Encoding: gzip, deflate
 * Connection: keep-alive
 * Upgrade-Insecure-Requests: 1
 *
 * 0,CONNECT FAIL
 * +STA_DISCONNECTED:"02:d2:42:8c:26:44"
 *
 */

#include "webserver.h"

// ------------------------------------------------------------ STATIC variables
// (configured in application.c)
extern const char *newLine;
extern osThreadId_t webserverThreadId;
extern const osThreadAttr_t webserver_attr;
extern osThreadId_t parserThreadId;
extern osSemaphoreId_t httpSem;
extern char *http_data;

// ------------------------------------------------------------ TASKS

void StartWebserverModule(void *argument) {
	webserver_module_t *module = (webserver_module_t*)argument;
	webserverThreadId = osThreadNew(Webserver, module, &webserver_attr);
	osThreadExit();
}

void Webserver(void *argument) {
	webserver_module_t module = *(webserver_module_t*)argument;
	uint32_t flags = 0;
	WEBSERVER_RETVAL ret;
	while(1) {
		flags = osThreadFlagsWait(NEW_CONNECTION|NEW_REQUEST, osFlagsWaitAny, osWaitForever);
		if(flagErrorHandler(flags)) {
			// On flag error, continue loop from beginning
			continue;
		}
		if(flags & NEW_CONNECTION) {
			ret = webserver_handle_connection();
			if(webserver_failureHandler(ret)) {
				printf_("WEBSERVER: error parsing input: 0x%02x%s", ret, newLine);
			}
		}
		if(flags & NEW_REQUEST) {
			ret = webserver_handle_request();
			if(webserver_failureHandler(ret)) {
				printf_("WEBSERVER: error parsing input: 0x%02x%s", ret, newLine);
			}
		}
	}
}

// ------------------------------------------------------------ PRIVATE

WEBSERVER_RETVAL webserver_handle_connection() {
	return WEBSERVER_NOT_IMPLEMENTED;
}

WEBSERVER_RETVAL webserver_handle_request() {
	return WEBSERVER_NOT_IMPLEMENTED;
}

bool webserver_failureHandler(WEBSERVER_RETVAL value) {
	switch(value) {
	case WEBSERVER_OK:
		return false;
	case WEBSERVER_FAILED_COPY:
		printf_("WEBSERVER: Failed copy!%s", newLine);
		break;
	case WEBSERVER_FAILED_SEMAPHORE:
		printf_("WEBSERVER: Failed semaphore!%s", newLine);
		break;
	case WEBSERVER_FAILED_FLAG:
		printf_("WEBSERVER: Failed flag!%s", newLine);
		break;
	case WEBSERVER_FAILED_QUEUE:
		printf_("WEBSERVER: Failed queue!%s", newLine);
		break;
	case WEBSERVER_MEM_ERROR:
		printf_("WEBSERVER: Out of memory!%s", newLine);
		break;
	case WEBSERVER_CONNECTION_ERROR:
		printf_("WEBSERVER: Connection error!%s", newLine);
		break;
	case WEBSERVER_REQUEST_ERROR:
		printf_("WEBSERVER: Request error!%s", newLine);
		break;
	case WEBSERVER_UNKOWN_ERROR:
		printf_("WEBSERVER: Unknown error!%s", newLine);
		break;
	case WEBSERVER_INIT_ERROR:
		printf_("WEBSERVER: Initialization error!%s", newLine);
		break;
	}
	return true;
}

// ------------------------------------------------------------ Webserver functions
