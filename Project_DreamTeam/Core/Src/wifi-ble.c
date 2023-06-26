/*
 * wifi-ble.c
 *
 *  Created on: May 22, 2023
 *      Author: rini
 */

#include "wifi-ble.h"

//#define WIFIBLE_DEBUG

// ------------------------------------------------------------ STATIC variables
// (configured in application.c)
extern const char *newLine;
extern const char *HTTP_HEADER;
extern const char *HTTP_CONTENT_TYPE_HTML;
extern const char *HTTP_CONTENT_TYPE_TEXT;
extern const char *HTTP_CONTENT_TYPE_JSON;
extern const char *HTTP_CONTENT_LENGTH_TEMPLATE;
extern const char *HTML_TEMPLATE;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern osThreadId_t wifibleThreadId;
extern const osThreadAttr_t wifible_attr;
extern osThreadId_t wifibleReceiverThreadId;
extern const osThreadAttr_t wifibleReceiver_attr;
extern osThreadId_t parserThreadId;
extern uint8_t uart1Buffer[WIFIBLE_UART_BUFFER_SIZE];
extern char *newMsg;
extern osSemaphoreId_t msgSem;
extern char *input;
extern osSemaphoreId_t inputSem;
extern uint8_t responseLink;
extern osSemaphoreId_t i2cSem;
extern uint8_t *heartrate_ram;
extern uint8_t heartrate_read_index;
extern uint8_t clients[4];
extern uint8_t requestType;
#ifndef USE_WEBSERVER
extern char update[MAX_UPDATE_SIZE];
extern char update_response[MAX_UPDATE_SIZE];
extern char page[MAX_PAGE_SIZE];
extern char page_response[MAX_PAGE_SIZE];
extern uint16_t page_response_length;
#endif

/*
const char *ip_address;
const char *port;
*/

// ------------------------------------------------------------ Interrupt routines

/* NOTE: These interrupt routines are not used or called
 * They were modified to provide debugging information in case of an error during development
 */

void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart) {
#ifdef DEBUG
	printf_("ISR: HAL_UART_AbortCpltCallback%s", newLine);
#endif
}

void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart) {
#ifdef DEBUG
	printf_("ISR: HAL_UART_AbortReceiveCpltCallback%s", newLine);
#endif
	if(huart->Instance == USART1) {
		/* NOTE: Retry reception routine
		HAL_StatusTypeDef ret = HAL_UART_Receive_DMA(&huart1, uart1Buffer, WIFIBLE_UART_BUFFER_SIZE);
		if(halStatusHandler(ret)) {
			printf_("ISR: error in DMA peripherals: ");
			translate_UART_Error(&huart1);
			return WIFIBLE_RESOURCE_ERROR;
		}
		*/
	}
}

// ------------------------------------------------------------ TASKS

void StartWifiBleModule(void *argument) {
	// parse argument
	// depending on argument, either initialize WiFi or Bluetooth
	// depending on argument, start the server or beacon
	wifible_module_t module = *(wifible_module_t*)argument;

	if(module.mode == WIFIBLE_WIRELESS_MODE) {
		// Initialize wifi
		wifible_wifi_cfg_t *module_config = wifible_wifi_default_cfg();
		wifibleThreadId = osThreadNew(WifiReceiver, NULL, &wifible_attr);
		wifible_prep_peripherals();
#ifndef WIFIBLE_SKIP_CONFIGURATION
		wifible_wifi_initialization(module_config);
#endif

	} else if(module.mode == WIFIBLE_BLUETOOTH_MODE) {
		// Initialize bluetooth
		wifible_bt_cfg_t *module_config = wifible_bt_default_cfg();
		wifibleThreadId = osThreadNew(BluetoothBeacon, NULL, &wifible_attr);
		wifible_prep_peripherals();
#ifndef WIFIBLE_SKIP_CONFIGURATION
		wifible_bt_initialization(module_config);
#endif
	}
	osThreadExit(); // Destroy task
}

void WifiReceiver(void *argument) {
	uint32_t flags;
	static size_t old_pos = WIFIBLE_UART_BUFFER_SIZE;  // Track the position of last character processed
	size_t reattempt_pos1 = 0;
	size_t reattempt_pos2 = 0;
	WIFIBLE_RETVAL ret;
	while(1) {
		// Wait for thread flags
		flags = osThreadFlagsWait(IDLE_EVENT|COPY_REATTEMPT|USER_INPUT|NEW_RESPONSE, osFlagsWaitAny, osWaitForever);
		if(flagErrorHandler(flags)) {
			// On flag error, continue loop from beginning
			continue;
		}
		// Wifi Receiver (USART1) State-Machine
		if(flags & IDLE_EVENT) {
			/* NOTE: Verifying DMA state (for debugging only)
			HAL_DMA_StateTypeDef dma_state = HAL_DMA_GetState(&hdma_usart1_rx);
			if(dma_state != HAL_DMA_STATE_BUSY) {
				printf_("WIFIBLE: DMA stopped: gState == 0x%08lx RxState == 0x%08lx%s", huart1.gState, huart1.RxState, newLine);
			}
			*/
			size_t new_pos = WIFIBLE_UART_BUFFER_SIZE - huart1.hdmarx->Instance->CNDTR;  // Compute the new position in the buffer
			if(new_pos != old_pos)  // Check if any new data is received
			{
				ret = wifible_handle_newData(old_pos, new_pos);
				if(wifible_failureHandler(ret) && ret == WIFIBLE_FAILED_COPY) {
					osThreadFlagsSet(parserThreadId, COPY_REATTEMPT);
					reattempt_pos1 = old_pos;
					reattempt_pos2 = new_pos;
				}
				old_pos = new_pos;  // Update the position of the last character processed
			}
		}
		if(flags & COPY_REATTEMPT) {
			ret = wifible_handle_newData(reattempt_pos1, reattempt_pos2);
			if(wifible_failureHandler(ret)) {
				printf_("WIFIBLE: 2nd copy-attempt failed: 0x%02x%s", ret, newLine);
			}
			reattempt_pos1 = 0;
			reattempt_pos2 = 0;
		}
		if(flags & USER_INPUT) {
			ret = wifible_handle_userInput();
			if(wifible_failureHandler(ret)) {
				printf_("WIFIBLE: Handling user input failed: 0x%02x%s", ret, newLine);
			}
		}
		if(flags & NEW_RESPONSE) {
			ret = wifible_handle_newRequest();
			if(wifible_failureHandler(ret)) {
				printf_("WIFIBLE: Handling new request failed: 0x%02x%s", ret, newLine);
			}
		}
	}
}

void BluetoothBeacon(void *argument) {
	Error_Handler(); // Not-implemented
}

// ------------------------------------------------------------ PRIVATE

wifible_wifi_cfg_t* wifible_wifi_default_cfg() {
	wifible_wifi_cfg_t *config = (wifible_wifi_cfg_t*) pvPortMalloc(sizeof(wifible_wifi_cfg_t));
	wifible_wifi_cfg_t default_cfg = {
			.ssid = "HEARTMONITOR",
			.pwd = "",
			.local_ip = "192.1.0.1",
			.local_gw = "192.1.0.1",
			.local_netmask = "255.255.255.0",
			.hostname = "heartmonitor",
			.domain = "monitor.heartrate.local",
			.mode = SoftAP
	};
    if (config == NULL) {  // Check if memory was allocated
        return NULL;  // Memory allocation failed
    }
    // Allocate config data
	config->ssid = pvPortMalloc(sizeof(char)*strlen(default_cfg.ssid)+1);
	sprintf_(config->ssid, "%s", default_cfg.ssid);
	config->pwd = pvPortMalloc(sizeof(char)*strlen(default_cfg.pwd)+1);
	sprintf_(config->pwd, "%s", default_cfg.pwd);
	config->local_ip = pvPortMalloc(sizeof(char)*strlen(default_cfg.local_ip)+1);
	sprintf_(config->local_ip, "%s", default_cfg.local_ip);
	config->local_gw = pvPortMalloc(sizeof(char)*strlen(default_cfg.local_gw)+1);
	sprintf_(config->local_gw, "%s", default_cfg.local_gw);
	config->local_netmask = pvPortMalloc(sizeof(char)*strlen(default_cfg.local_netmask)+1);
	sprintf_(config->local_netmask, "%s", default_cfg.local_netmask);
	config->hostname = pvPortMalloc(sizeof(char)*strlen(default_cfg.hostname)+1);
	sprintf_(config->hostname, "%s", default_cfg.hostname);
	config->domain = pvPortMalloc(sizeof(char)*strlen(default_cfg.domain)+1);
	sprintf_(config->domain, "%s", default_cfg.domain);
	config->mode = SoftAP;

	// Setup HTTP response variables
	char header[128] = {0};
	sprintf_(header, "%s%s", HTTP_HEADER, HTTP_CONTENT_TYPE_HTML);
	uint16_t header_length = strlen(header);
	sprintf_(page, HTML_TEMPLATE, config->local_ip, "80");
	uint16_t page_length = strlen(page);
	sprintf_(header+header_length, HTTP_CONTENT_LENGTH_TEMPLATE, page_length);
	sprintf_(page_response, "%s%s", header, page);
	page_response_length = strlen(page_response);
	return config;
}

WIFIBLE_RETVAL wifible_wifi_clear_cfg(wifible_wifi_cfg_t *cfg) {
	vPortFree(cfg->ssid);
	vPortFree(cfg->pwd);
	vPortFree(cfg->local_ip);
	vPortFree(cfg->local_gw);
	vPortFree(cfg->local_netmask);
	vPortFree(cfg->hostname);
	vPortFree(cfg->domain);
	vPortFree(cfg);
	return WIFIBLE_OK;
}

WIFIBLE_RETVAL wifible_wifi_initialization(wifible_wifi_cfg_t *cfg) {
	char at_cmd_buf[AT_CMD_BUFFER_SIZE] = {0};
	//uint8_t ret = 0;
	// TODO Verify responses in wifi initialization
	printf_("--------------- Starting wifi configuration...%s", newLine);
	// --------------- WiFi Configuration ---------------
	// Configure SoftAP Mode
	at_set_command(at_cmd_buf, wifible_send_command, AT_WIFI_Set_Mode, "%u",
			AT_WIFI_SoftAP_Mode);
	osDelay(WIFIBLE_CMD_DELAY);
	// SSID=Config, PWD=Config, Channel=1, ECN=Open, MaxConn=2, Broadcast=0
	at_set_command(at_cmd_buf, wifible_send_command, AT_WIFI_Set_SoftAP_Config, "\"%s\",\"%s\",%u,%u,%u,%u",
			cfg->ssid, cfg->pwd, 1, AT_WIFI_Encryption_OPEN, 2, 0);
	osDelay(WIFIBLE_CMD_DELAY);
	// Enable DHCP Server
	at_set_command(at_cmd_buf, wifible_send_command, AT_WIFI_Enable_DHCP, "%u,%u",
				0, 3);
	/*
	at_set_command(at_cmd_buf, wifible_send_command, AT_WIFI_Enable_DHCP, "%u,%u",
			AT_Disable, AT_WIFI_Mask_Enable_SoftAP_DHCP);
	*/
	osDelay(WIFIBLE_CMD_DELAY);
	// Configure DHCP Range
	at_set_command(at_cmd_buf, wifible_send_command, AT_WIFI_Set_DHCP_Range, "%u,%u,\"%s\",\"%s\"",
			AT_Enable, 2880, "192.1.0.20", "192.1.0.30");
	osDelay(WIFIBLE_CMD_DELAY);
	// Set hostname
	at_set_command(at_cmd_buf, wifible_send_command, AT_WIFI_Set_Hostname, "\"%s\"",
			cfg->hostname);
	osDelay(WIFIBLE_CMD_DELAY);
	// Configure MDNS
	at_set_command(at_cmd_buf, wifible_send_command, AT_WIFI_MDNS, "%u",
			AT_Disable);
			//"%u,\"%s\",\"%s\",%u" => AT_Enable, config->hostname, "_heartrate", 8080
	osDelay(WIFIBLE_CMD_DELAY);

	printf_("--------------- Starting TCP/IP configuration...%s", newLine);
	// --------------- TCP/IP Configuration ---------------
	// Set IP address of module
	at_set_command(at_cmd_buf, wifible_send_command, AT_IP_Set_SoftAP_IP, "\"%s\",\"%s\",\"%s\"",
			cfg->local_ip, cfg->local_gw, cfg->local_netmask);
	osDelay(WIFIBLE_CMD_DELAY);
	// Allow multiple connections (for server)
	at_set_command(at_cmd_buf, wifible_send_command, AT_IP_Set_MultiConnectionMode, "%u",
			AT_IP_ConnectionMode_Multiple);
	osDelay(WIFIBLE_CMD_DELAY);
	// Start TCP server on Port 80
	at_set_command(at_cmd_buf, wifible_send_command, AT_IP_Server, "%u,%u",
			AT_IP_Server_Create, 80);
	osDelay(WIFIBLE_CMD_DELAY);
	at_execute_command(at_cmd_buf, wifible_send_command, AT_Startup);
	osDelay(WIFIBLE_CMD_DELAY);

	printf_("--------------- Final click module information:%s", newLine);
	at_execute_command(at_cmd_buf, wifible_send_command, AT_Version);
	osDelay(WIFIBLE_CMD_DELAY);
	at_query_command(at_cmd_buf, wifible_send_command, AT_IP_Set_SoftAP_IP);
	osDelay(WIFIBLE_CMD_DELAY);
	at_query_command(at_cmd_buf, wifible_send_command, AT_IP_Set_SoftAP_MAC);
	osDelay(WIFIBLE_CMD_DELAY);
	return WIFIBLE_OK;
}

wifible_bt_cfg_t* wifible_bt_default_cfg() {
	return NULL; // Not implemented
}

WIFIBLE_RETVAL wifible_bt_clear_cfg(wifible_bt_cfg_t *cfg) {
	return WIFIBLE_NOT_IMPLEMENTED;
}

WIFIBLE_RETVAL wifible_bt_initialization(wifible_bt_cfg_t *cfg) {
	return WIFIBLE_NOT_IMPLEMENTED;
}

WIFIBLE_RETVAL wifible_power_module(uint8_t power_state) {
	// TODO Implement WiFi-BLE Power function
	return WIFIBLE_NOT_IMPLEMENTED;
}

WIFIBLE_RETVAL wifible_handle_newData(size_t old_pos, size_t new_pos) {
	size_t length;
#ifdef WIFIBLE_DEBUG
	HAL_StatusTypeDef ret;
#endif
	// The transmissions here are in blocking mode, otherwise output is cutoff!
	// If received data is BIGGER than buffer size, then output will be cutoff! (=> Increase buffer size)
	// The function does not abort on transmission error, but will abort on copy-error
	if (new_pos > old_pos)  // If data does not wrap around the buffer
	{
		length = new_pos - old_pos;
		// Transmit data
#ifdef WIFIBLE_DEBUG
		ret = HAL_UART_Transmit(&huart2, &uart1Buffer[old_pos], length, HAL_MAX_DELAY);
		if(halStatusHandler(ret)) {
			printf_("WIFIBLE: ");
			translate_UART_Error(&huart2);
		}
#endif
		// Process your data => uart1Buffer[old_pos] TO uart1Buffer[old_pos+length] == Received DATA
		return wifible_attempt_fullCopy(old_pos, length);
	}
	else  // If data wraps around the buffer
	{
		// If you process data in here, you'll need to partially construct your data

		// First transmit the data until the end of the buffer
		length = WIFIBLE_UART_BUFFER_SIZE - old_pos;
		size_t tmp = length;
#ifdef WIFIBLE_DEBUG
		ret = HAL_UART_Transmit(&huart2, &uart1Buffer[old_pos], length, HAL_MAX_DELAY);
		if(halStatusHandler(ret)) {
			printf_("WIFIBLE: ");
			translate_UART_Error(&huart2);
		}
#endif
		// Then transmit the remaining data from the beginning of the buffer
		length = new_pos;
#ifdef WIFIBLE_DEBUG
		ret = HAL_UART_Transmit(&huart2, uart1Buffer, length, HAL_MAX_DELAY);
		if(halStatusHandler(ret)) {
			printf_("WIFIBLE: ");
			translate_UART_Error(&huart2);
		}
#endif
		// Process data partially
		return wifible_attempt_partialCopy(old_pos, tmp, length);
	}
}

/* NOTE: Nearly identical implementations
 * The functions "wifible_attempt_fullCopy" and "wifible_attempt_partialCopy" are almost identical in nature
 * I decided to keep it that way for time being.
 * The following needs to be kept in mind when changing the functions:
 *  - Anything before "START OF COPY" and after "END OF COPY" must be kept identical
 * This may change in the future, this note should serve as a reminder
 */

// Custom enter method for clearing buffer
WIFIBLE_RETVAL wifible_enter_copy() {
	if(acquire_semaphore(msgSem, MODULE_SEM_TIMEOUT) != WIFIBLE_OK) {
		return WIFIBLE_FAILED_COPY;
	}
	// Clear any previously allocated buffer of newMsg
	vPortFree(newMsg); // The case (newMsg == NULL) is ignored by heap4.c
	newMsg = NULL; // Reset pointer
	return WIFIBLE_OK;
}

WIFIBLE_RETVAL wifible_attempt_fullCopy(size_t start_pos, size_t length) {
	uint32_t flag_return;
	WIFIBLE_RETVAL wifible_return;
	if((wifible_return = wifible_enter_copy()) != WIFIBLE_OK) {
		return wifible_return;
	}
	// -------------------------------- START OF COPY
	// Request new heap memory for received data
	newMsg = pvPortMalloc(sizeof(char)*length+1);
	if(newMsg == NULL) {
		return WIFIBLE_MEM_ERROR;
	}
	// Copy data
	memcpy(newMsg, uart1Buffer+start_pos, length);
	newMsg[length] = '\0';
	// -------------------------------- END OF COPY
	if((wifible_return = release_semaphore(msgSem)) != WIFIBLE_OK) {
		return wifible_return;
	}
	// Notify parser about new message
	flag_return = osThreadFlagsSet(parserThreadId, MSG_EVENT);
	if(flagErrorHandler(flag_return)) {
		return WIFIBLE_FAILED_FLAG;
	}
	return WIFIBLE_OK;
}

WIFIBLE_RETVAL wifible_attempt_partialCopy(size_t start_pos, size_t len1, size_t len2) {
	uint32_t flag_return;
	WIFIBLE_RETVAL wifible_return;
	if((wifible_return = wifible_enter_copy()) != WIFIBLE_OK) {
		return wifible_return;
	}
	// -------------------------------- START OF COPY
	// Request heap memory for data
	newMsg = pvPortMalloc(sizeof(char)*(len1+len2)+1);
	if(newMsg == NULL) {
		return WIFIBLE_MEM_ERROR;
	}
	// copy 1st part of data
	memcpy(newMsg, uart1Buffer+start_pos, len1);
	// copy 2nd part of data
	memcpy(newMsg+len1, uart1Buffer, len2);
	newMsg[len1+len2] = '\0';
	// -------------------------------- END OF COPY
	if((wifible_return = release_semaphore(msgSem)) != WIFIBLE_OK) {
		return wifible_return;
	}
	// Notify parser about new message
	flag_return = osThreadFlagsSet(parserThreadId, MSG_EVENT);
	if(flagErrorHandler(flag_return)) {
		return WIFIBLE_FAILED_FLAG;
	}
	return WIFIBLE_OK;
}

WIFIBLE_RETVAL wifible_handle_userInput() {
	char at_cmd_buf[AT_CMD_BUFFER_SIZE] = {0};
	WIFIBLE_RETVAL wifible_return;
	// Check semaphore if input is available
	if((wifible_return = acquire_semaphore(inputSem, MODULE_SEM_TIMEOUT)) != WIFIBLE_OK) {
		return wifible_return;
	}
	// Forward input to WIFIBLE module
	at_execute_command(at_cmd_buf, wifible_send_command, input);
	// NOTE: input Buffer is free'd by CONSOLE module
	// Release semaphore to free input
	if((wifible_return = release_semaphore(inputSem)) != WIFIBLE_OK) {
		return wifible_return;
	}
	return WIFIBLE_OK;
}

WIFIBLE_RETVAL wifible_handle_newRequest() {
	WIFIBLE_RETVAL wifible_return;
	/*
	if((wifible_return = acquire_semaphore(i2cSem, osWaitForever)) != WIFIBLE_OK) {
		return wifible_return;
	}
	*/
	// Get data to display on webpage
	/*
	int data = heartrate_ram[heartrate_read_index++];
	if(heartrate_read_index >= HEARTRATE_RAM_SIZE) {
		heartrate_read_index = 0;
	}
	*/
	wifible_return = wifible_serve_webPage(responseLink);
	return wifible_return;
}

WIFIBLE_RETVAL wifible_serve_webPage(uint8_t link_id) {
	char at_cmd_buf[AT_CMD_BUFFER_SIZE] = {0};
	uint8_t heartrate = heartrate_ram[0];
	uint8_t heartrate_calc = heartrate_ram[1];

	if(requestType == JSON_REQUEST) {
		// Client already loaded page and requests data update
		char header[128] = {0};
		sprintf_(header, "%s%s", HTTP_HEADER, HTTP_CONTENT_TYPE_JSON);
		uint16_t header_length = strlen(header);
		// FIXME SOMEHOW this line FUCKS up the values, even though there is no conversion
		sprintf_(update, "{\"HR\":%u,\"HRCALC\":%u}%s", heartrate, heartrate_calc, newLine);
		printf_("WIFIBLE: %s", update);
		uint16_t update_length = strlen(update);
		sprintf_(header+header_length, HTTP_CONTENT_LENGTH_TEMPLATE, update_length);
		sprintf_(update_response, "%s%s", header, update);
		uint16_t update_response_length = strlen(update_response);
		at_set_command(at_cmd_buf, wifible_send_command, AT_IP_Send, "%hhu,%hu", link_id, update_response_length);
		osDelay(WIFIBLE_CMD_DELAY);
		HAL_StatusTypeDef hal_return = HAL_UART_Transmit_DMA(&huart1, (uint8_t*)update_response, update_response_length);
		if(halStatusHandler(hal_return)) {
			printf_("WIFIBLE: error serving webpage: ");
			translate_UART_Error(&huart1);
			return WIFIBLE_RW_ERROR;
		}
	} else {
		// Client requests new webpage
		// Start Send command
		at_set_command(at_cmd_buf, wifible_send_command, AT_IP_Send, "%hhu,%hu", link_id, page_response_length);
		osDelay(WIFIBLE_CMD_DELAY);
		// Send data
		HAL_StatusTypeDef hal_return = HAL_UART_Transmit_DMA(&huart1, (uint8_t*)page_response, page_response_length);
		if(halStatusHandler(hal_return)) {
			printf_("WIFIBLE: error serving webpage: ");
			translate_UART_Error(&huart1);
			return WIFIBLE_RW_ERROR;
		}
#ifdef WIFIBLE_DEBUG
		printf_("WIFIBLE: Sent:%s%s%s", newLine, page_response, newLine);
#endif
	}
	// Wait until TC flag is set
	while(!(huart1.Instance->ISR & USART_ISR_TC));
	// Just close all the connections
	at_set_command(at_cmd_buf, wifible_send_command, AT_IP_CloseConnection, "%hhu", 5);
	osDelay(WIFIBLE_CMD_DELAY);
	return WIFIBLE_OK;
}

void wifible_send_command(char *command, int length) {
	HAL_StatusTypeDef ret = HAL_UART_Transmit_DMA(&huart1, (uint8_t*)command, length);
	if(halStatusHandler(ret)) {
		printf_("WIFIBLE: error sending command: %s%s", command, newLine);
		translate_UART_Error(&huart1);
	}
	// Wait until TC flag is set
	while(!(huart1.Instance->ISR & USART_ISR_TC));
}

void wifible_uart_receive_callback(UART_HandleTypeDef *huart) {
#ifdef WIFIBLE_DEUBG
	printf_("WIFIBLE: UART callback%s", newLine);
#endif
}

void wifible_dma_receive_callback(DMA_HandleTypeDef *hdma) {
#ifdef WIFIBLE_DEUBG
	printf_("WIFIBLE: DMA callback%s", newLine);
#endif
}

WIFIBLE_RETVAL wifible_prep_peripherals() {
	// Activate UART interrupts and reception
	LL_USART_EnableIT_IDLE(USART1); // Enable idle line detection (interrupt) for uart1
	// NOTE: Please check stm32l4xx_it.c & usart.c for the USER-CODE that handles the IDLE Line Interrupt!!
	HAL_StatusTypeDef ret = HAL_UART_Receive_DMA(&huart1, uart1Buffer, WIFIBLE_UART_BUFFER_SIZE);

	//HAL_StatusTypeDef ret = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1Buffer, WIFIBLE_UART_BUFFER_SIZE);
	//huart1.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
	if(halStatusHandler(ret)) {
		printf_("WIFIBLE: error in DMA peripherals: ");
		translate_UART_Error(&huart1);
		return WIFIBLE_RESOURCE_ERROR;
	}
	//HAL_DMA_RegisterCallback(&hdma_usart1_rx, HAL_DMA_XFER_CPLT_CB_ID, wifible_dma_receive_callback);
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
	HAL_UART_RegisterCallback(&huart1, HAL_UART_RX_COMPLETE_CB_ID, wifible_uart_receive_callback);
#endif
	return WIFIBLE_OK;
}

bool wifible_failureHandler(WIFIBLE_RETVAL value) {
	switch(value) {
	case WIFIBLE_OK:
		return false;
	case WIFIBLE_FAILED_COPY:
		printf_("WIFIBLE: Failed copy!%s", newLine);
		break;
	case WIFIBLE_FAILED_SEMAPHORE:
		printf_("WIFIBLE: Failed semaphore!%s", newLine);
		break;
	case WIFIBLE_FAILED_FLAG:
		printf_("WIFIBLE: Failed flag!%s", newLine);
		break;
	case WIFIBLE_MEM_ERROR:
		printf_("WIFIBLE: Out of Memory!%s", newLine);
		break;
	case WIFIBLE_RESOURCE_ERROR:
		printf_("WIFIBLE: Resource error!%s", newLine);
		break;
	case WIFIBLE_RW_ERROR:
		printf_("WIFIBLE: Read/Write error!%s", newLine);
		break;
	case WIFIBLE_POWER_ERROR:
		printf_("WIFIBLE: Power error!%s", newLine);
		break;
	case WIFIBLE_INIT_ERROR:
		printf_("WIFIBLE: Initialization error!%s", newLine);
		break;
	default:
		printf_("WIFIBLE: Unknown error! (0x%02x)%s", value, newLine);
		break;
	}
	return true;
}
