/*
 * static.c
 *
 *  Created on: Jun 18, 2023
 *      Author: rini
 */

/* NOTE: This source file contains static data used throughout the application
 */

#include <application.h>
#include "cmsis_os2.h"
#include "parser.h"
#include "wifi-ble.h"
#include "console.h"
#include "webserver.h"

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

// ------------------------------------------------------------ WIFIBLE MODULE
const char *HTTP_HEADER = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const char *HTML_TEMPLATE = "<html><body><p id='data'>%d</p></body></html>\r\n";

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
webserver_module_t webserver_module = {
		.module_flags = 0
};
osThreadId_t webserverInitThreadId = NULL;
const osThreadAttr_t webserverInit_attr = {
	.name = WEBSERVER_MODULE_INIT_NAME,
	.priority = (osPriority_t) osPriorityBelowNormal,
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
osMessageQueueId_t clientPipe = NULL;
StaticQueue_t clientPipe_cb;
client_t clientPipe_mq[WEBSERVER_MAX_CONNECTIONS];
osMessageQueueAttr_t clientPipe_attr = {
	.name = "ClientPipe",
	.cb_mem = &clientPipe_cb,
	.cb_size = sizeof(StaticQueue_t),
	.mq_mem = clientPipe_mq,
	.mq_size = WEBSERVER_MAX_CONNECTIONS * sizeof(client_t)
};
osMessageQueueId_t requestPipe = NULL;
StaticQueue_t requestPipe_cb;
requestPacket_t requestPipe_mq[WEBSERVER_MAX_HANDLE_QUEUE];
osMessageQueueAttr_t requestPipe_attr = {
	.name = "RequestPipe",
	.cb_mem = &clientPipe_cb,
	.cb_size = sizeof(StaticQueue_t),
	.mq_mem = clientPipe_mq,
	.mq_size = WEBSERVER_MAX_HANDLE_QUEUE * sizeof(requestPacket_t)
};
osMessageQueueId_t responsePipe = NULL;
StaticQueue_t responsePipe_cb;
responsePacket_t responsePipe_mq[WEBSERVER_MAX_HANDLE_QUEUE];
osMessageQueueAttr_t responsePipe_attr = {
	.name = "ResponsePipe",
	.cb_mem = &clientPipe_cb,
	.cb_size = sizeof(StaticQueue_t),
	.mq_mem = clientPipe_mq,
	.mq_size = WEBSERVER_MAX_HANDLE_QUEUE * sizeof(responsePacket_t)
};

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

// ------------------------------------------------------------ APPLICATION INIT

void StartInitTask(void *argument) {
	printf_("Starting application...%s", newLine);
	inputSem = osSemaphoreNew(INPUT_MAX_COUNT, INPUT_INIT_COUNT, &inputSem_attr);
	msgSem = osSemaphoreNew(MESSAGE_MAX_COUNT, MESSAGE_INIT_COUNT, &msgSem_attr);
	clientPipe = osMessageQueueNew(WEBSERVER_MAX_CONNECTIONS, sizeof(client_t), &clientPipe_attr);
	requestPipe = osMessageQueueNew(WEBSERVER_MAX_HANDLE_QUEUE, sizeof(requestPacket_t), &requestPipe_attr);
	responsePipe = osMessageQueueNew(WEBSERVER_MAX_HANDLE_QUEUE, sizeof(responsePacket_t), &responsePipe_attr);
	parserInitThreadId = osThreadNew(StartParser, &parser_module, &parserInit_attr);
	threadErrorHandler(parserInitThreadId, PARSER_MODULE_INIT_NAME);
	wifibleInitThreadId = osThreadNew(StartWifiBleModule, &wifible_module, &wifibleInit_attr);
	threadErrorHandler(wifibleInitThreadId, WIFIBLE_MODULE_INIT_NAME);
	consoleInitThreadId = osThreadNew(StartConsoleInterface, &console_module, &consoleInit_attr);
	threadErrorHandler(consoleInitThreadId, CONSOLE_MODULE_INIT_NAME);
	webserverInitThreadId = osThreadNew(StartWebserverModule, &webserver_module, &webserverInit_attr);
	threadErrorHandler(webserverInitThreadId, WEBSERVER_MODULE_INIT_NAME);

	osThreadExit(); // destroy task
}
