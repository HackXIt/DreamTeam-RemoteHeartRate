/*
 * static.c
 *
 *  Created on: Jun 18, 2023
 *      Author: rini
 */

/* NOTE: This source file contains static data used throughout the application
 */

#include <application.h>
#include "parser.h"
#include "wifi-ble.h"
#include "console.h"
#include "webserver.h"
#include "heartrateReceiver.h"

// ------------------------------------------------------------ CONSTANT APPLICATION DATA
const char *newLine = "\r\n";

// ------------------------------------------------------------ PARSER MODULE
parser_module_t parser_module = {
	.module_flags = 0
};
osThreadId_t parserInitThreadId = NULL;
const osThreadAttr_t parserInit_attr = {
	.name = PARSER_MODULE_INIT_NAME,
	.priority = (osPriority_t) osPriorityBelowNormal1,
	.stack_size = MODULE_INIT_SIZE * 4,
};
osThreadId_t parserThreadId = NULL;
uint32_t parserThreadStack[PARSER_MODULE_STACK_SIZE];
StaticTask_t parserThreadTCB;
const osThreadAttr_t parser_attr = {
	.name = PARSER_MODULE_NAME,
	.priority = (osPriority_t) osPriorityAboveNormal,
	.stack_size = PARSER_MODULE_STACK_SIZE * 4,
	.stack_mem = parserThreadStack,
	.cb_mem = &parserThreadTCB,
	.cb_size = sizeof(StaticTask_t)
};
char request_buffer[PARSER_REQUEST_BUFFER_SIZE] = {0};
char response_buffer[PARSER_RESPONSE_BUFFER_SIZE] = {0};

// ------------------------------------------------------------ WIFIBLE MODULE
// 19 characters
const char *HTTP_HEADER = "HTTP/1.1 200 OK\r\n";
// 27 characters
// Client should fetch GET with "Connection: close"
const char *HTTP_CONTENT_TYPE_HTML = "Content-type: text/html\r\n";
const char *HTTP_CONTENT_TYPE_TEXT = "Content-type: text/plain\r\n";
const char *HTTP_CONTENT_TYPE_JSON = "Content-type: application/json\r\n";
// 26 characters without placeholder
const char *HTTP_CONTENT_LENGTH_TEMPLATE = "Content-Length: %hu\r\nCache-Control: no-cache, must-revalidate\r\n\r\n";
#ifdef WIFIBLE_USE_WEBSOCKET
// 212 characters without placeholders
const char *HTML_TEMPLATE = "<!DOCTYPE html><html><body><p id='data' style='font-size: 50vw;'>%hhu</p><script>var ws = new WebSocket('ws://%s:%s');ws.onmessage = function(event) {var dataElement = document.getElementById('data');dataElement.innerHTML = event.data;};</script></body></html>";
#endif
#ifndef WIFIBLE_USE_WEBSOCKET
// 378 characters without placeholders
const char *HTML_TEMPLATE = "<!DOCTYPE html><html><head><title>HEARTMONITOR</title><link rel='icon' href='data:,'></head><body><p id='data' style='font-size: 10vw;'>%hhu</p><script>let updatePage = async () => { let response = await fetch('http://%s:%s', { method: 'GET', headers: { 'Accept': 'application/json'}, }); if(response.ok){ let ret = await response.json(); document.getElementById('data').innerText = ret.DATA; } else { console.error('HTTP error: ' + response.status); }};setInterval(updatePage, 2000);</script></body></html>";
#endif


wifible_module_t wifible_module = {
	.mode = WIFIBLE_WIRELESS_MODE
};
osThreadId_t wifibleInitThreadId = NULL;
const osThreadAttr_t wifibleInit_attr = {
	.name = WIFIBLE_MODULE_INIT_NAME,
	.priority = (osPriority_t) osPriorityBelowNormal2,
	.stack_size = MODULE_INIT_SIZE * 4,
};
osThreadId_t wifibleThreadId = NULL;
uint32_t wifibleThreadStack[WIFIBLE_MODULE_STACK_SIZE];
StaticTask_t wifibleThreadTCB;
const osThreadAttr_t wifible_attr = {
	.name = WIFIBLE_MODULE_NAME,
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = WIFIBLE_MODULE_STACK_SIZE * 4,
	.stack_mem = wifibleThreadStack,
	.cb_mem = &wifibleThreadTCB,
	.cb_size = sizeof(StaticTask_t)
};
uint8_t uart1Buffer[WIFIBLE_UART_BUFFER_SIZE];
char *newMsg = NULL;
uint8_t responseLink = 0;
uint8_t requestType = 0;
#ifndef USE_WEBSERVER
uint8_t clients[4] = {0};
char page[1024] = {0};
char response[1024] = {0};
#endif

// ------------------------------------------------------------ CONSOLE MODULE
console_module_t console_module = {
	.module_flags = FEEDBACK_ON|INPUT_ON
};
osThreadId_t consoleInitThreadId = NULL;
const osThreadAttr_t consoleInit_attr = {
	.name = CONSOLE_MODULE_INIT_NAME,
	.priority = (osPriority_t) osPriorityBelowNormal3,
	.stack_size = MODULE_INIT_SIZE * 4,
};
osThreadId_t consoleThreadId = NULL;
uint32_t consoleThreadStack[CONSOLE_MODULE_STACK_SIZE];
StaticTask_t consoleThreadTCB;
const osThreadAttr_t console_attr = {
	.name = CONSOLE_MODULE_NAME,
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = CONSOLE_MODULE_STACK_SIZE * 4,
	.stack_mem = consoleThreadStack,
	.cb_mem = &consoleThreadTCB,
	.cb_size = sizeof(StaticTask_t)
};
uint8_t uart2Buffer[CONSOLE_UART_BUFFER_SIZE] = {0};
uint16_t uart2BufferIndex = 0;
uint8_t rx_byte = 0;
char *input = NULL;

// ------------------------------------------------------------ WEBSERVER MODULE
#ifdef USE_WEBSERVER
webserver_module_t webserver_module = {
		.module_flags = 0
};
osThreadId_t webserverInitThreadId = NULL;
const osThreadAttr_t webserverInit_attr = {
	.name = WEBSERVER_MODULE_INIT_NAME,
	.priority = (osPriority_t) osPriorityBelowNormal4,
	.stack_size = MODULE_INIT_SIZE * 4,
};
osThreadId_t webserverThreadId = NULL;
uint32_t webserverThreadStack[WEBSERVER_MODULE_STACK_SIZE];
StaticTask_t webserverThreadTCB;
const osThreadAttr_t webserver_attr = {
	.name = WEBSERVER_MODULE_NAME,
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = WEBSERVER_MODULE_STACK_SIZE * 4,
	.stack_mem = webserverThreadStack,
	.cb_mem = &webserverThreadTCB,
	.cb_size = sizeof(StaticTask_t)
};
char *http_data = NULL;
static const char *s_listen_on = "ws://localhost:80";
static const char *s_web_root = ".";
#endif

// ------------------------------------------------------------ HEARTRATE RECEIVER MODULE
hrReceiver_module_t hrReceiver_module = {
		.module_flags = 0
};
osThreadId_t hrReceiverInitThreadId = NULL;
const osThreadAttr_t hrReceiverInit_attr = {
	.name = HRRECEIVER_MODULE_INIT_NAME,
	.priority = (osPriority_t) osPriorityBelowNormal5,
	.stack_size = MODULE_INIT_SIZE * 4,
};
osThreadId_t hrReceiverThreadId = NULL;
uint32_t hrReceiverThreadStack[HRRECEIVER_MODULE_STACK_SIZE];
StaticTask_t hrReceiverThreadTCB;
const osThreadAttr_t hrReceiver_attr = {
	.name = HRRECEIVER_MODULE_NAME,
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = HRRECEIVER_MODULE_STACK_SIZE * 4,
	.stack_mem = hrReceiverThreadStack,
	.cb_mem = &hrReceiverThreadTCB,
	.cb_size = sizeof(StaticTask_t)
};
uint8_t heartrate_ram[HEARTRATE_RAM_SIZE] = {0};
uint8_t heartrate_rx_offset = 0;
uint8_t heartrate_read_index = 0;

// ------------------------------------------------------------ SHARED VARIABLES
osSemaphoreId_t inputSem = NULL;
StaticSemaphore_t inputSem_cb;
osSemaphoreAttr_t inputSem_attr = {
	.name = INPUT_SEMAPHORE,
	.cb_mem = &inputSem_cb,
	.cb_size = sizeof(StaticSemaphore_t)
};
osSemaphoreId_t msgSem = NULL;
StaticSemaphore_t msgSem_cb;
osSemaphoreAttr_t msgSem_attr = {
	.name = MESSAGE_SEMAPHORE,
	.cb_mem = &msgSem_cb,
	.cb_size = sizeof(StaticSemaphore_t)
};
osSemaphoreId_t httpSem = NULL;
StaticSemaphore_t httpSem_cb;
osSemaphoreAttr_t httpSem_attr = {
	.name = HTTP_SEMAPHORE,
	.cb_mem = &httpSem_cb,
	.cb_size = sizeof(StaticSemaphore_t)
};
osSemaphoreId_t i2cSem = NULL;
StaticSemaphore_t i2cSem_cb;
osSemaphoreAttr_t i2cSem_attr = {
	.name = I2C_SEMAPHORE,
	.cb_mem = &i2cSem_cb,
	.cb_size = sizeof(StaticSemaphore_t)
};

// ------------------------------------------------------------ APPLICATION INIT

void StartInitTask(void *argument) {
	printf_("Starting application...%s", newLine);
	inputSem = osSemaphoreNew(INPUT_MAX_COUNT, INPUT_INIT_COUNT, &inputSem_attr);
	msgSem = osSemaphoreNew(MESSAGE_MAX_COUNT, MESSAGE_INIT_COUNT, &msgSem_attr);
	httpSem = osSemaphoreNew(HTTP_MAX_COUNT, HTTP_INIT_COUNT, &httpSem_attr);
	i2cSem = osSemaphoreNew(HEARTRATE_RAM_SIZE, 0, &i2cSem_attr);
	parserInitThreadId = osThreadNew(StartParser, &parser_module, &parserInit_attr);
	threadErrorHandler(parserInitThreadId, PARSER_MODULE_INIT_NAME);
	wifibleInitThreadId = osThreadNew(StartWifiBleModule, &wifible_module, &wifibleInit_attr);
	threadErrorHandler(wifibleInitThreadId, WIFIBLE_MODULE_INIT_NAME);
	consoleInitThreadId = osThreadNew(StartConsoleInterface, &console_module, &consoleInit_attr);
	threadErrorHandler(consoleInitThreadId, CONSOLE_MODULE_INIT_NAME);
#ifdef USE_WEBSERVER
	webserverInitThreadId = osThreadNew(StartWebserverModule, &webserver_module, &webserverInit_attr);
	threadErrorHandler(webserverInitThreadId, WEBSERVER_MODULE_INIT_NAME);
#endif
	hrReceiverInitThreadId = osThreadNew(StartHrReceiver, &hrReceiver_module, &hrReceiverInit_attr);
	threadErrorHandler(hrReceiverInitThreadId, HRRECEIVER_MODULE_INIT_NAME);


	osThreadExit(); // destroy task
}

APPLICATION_RETVAL acquire_semaphore(osSemaphoreId_t semaphore, uint32_t timeout) {
	osStatus_t os_return = osSemaphoreAcquire(semaphore, timeout);
	if(osStatusHandler(os_return)) {
		return SEMAPHORE_FAILURE;
	}
	return APPLICATION_OK;
}
APPLICATION_RETVAL release_semaphore(osSemaphoreId_t semaphore) {
	osStatus_t os_return = osSemaphoreRelease(semaphore);
	if(osStatusHandler(os_return)) {
		return SEMAPHORE_FAILURE;
	}
	return APPLICATION_OK;
}
