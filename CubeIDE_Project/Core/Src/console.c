/*
 * console.c
 *
 *  Created on: May 30, 2023
 *      Author: rini
 */


#include "console.h"

// ------------------------------------------------------------ STATIC variables
// (configured in application.c)
extern osThreadId_t consoleThreadId;
extern const osThreadAttr_t console_attr;
extern const char *newLine;
extern uint8_t uart2Buffer[CONSOLE_UART_BUFFER_SIZE];
extern uint16_t uart2BufferIndex;
extern uint8_t rx_byte;
extern osSemaphoreId_t inputSem;
extern osThreadId_t parserThreadId;
extern char *input;

// ------------------------------------------------------------ Interrupt routines

// Interrupt callback routine for UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2) {
	  // Notify task about idle event
	  uint32_t ret = osThreadFlagsSet(consoleThreadId, RX_BYTE);
	  if(flagErrorHandler(ret)) {
		  printf_("ISR: thread flag error: 0x%08lx%s", ret, newLine);
	  }

	  // Restart reception with interrupt
	  HAL_StatusTypeDef status = HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	  if(halStatusHandler(status)) {
		  printf_("ISR: HAL status error: 0x%02x%s", status, newLine);
	  }
  }
}

// ------------------------------------------------------------ TASKS

void StartConsoleInterface(void *argument) {
	console_module_t *module = (console_module_t*)argument;
	consoleThreadId = osThreadNew(ConsoleMonitor, module, &console_attr);
	CONSOLE_RETVAL ret = console_prep_peripherals();
	if(console_failureHandler(ret)) {
		console_failureHandler(CONSOLE_INIT_ERROR);
	}
	osThreadExit();
}

void ConsoleMonitor(void *argument) {
	console_module_t *module = (console_module_t*)argument;
	// switch-case state-machine handling thread flags
	uint32_t flags = 0;
	CONSOLE_RETVAL ret;
	while(1) {
		// Wait for thread flags
		flags = osThreadFlagsWait(RX_BYTE|BUFFER_OVERRUN, osFlagsWaitAny, osWaitForever);
		if(flagErrorHandler(flags)) {
			// On flag error, continue loop from beginning
			continue;
		}
		// Console (USART2) State-Machine
		if(flags & RX_BYTE) {
			ret = console_handle_rxByte();
			if(console_failureHandler(ret)) {
				printf_("CONSOLE: error handling received byte: 0x%02x%s", ret, newLine);
			} else if (ret == CONSOLE_NEW_DATA) {
				ret = console_handle_newInput();
				if(console_failureHandler(ret)) {
					printf_("CONSOLE: error handling new input: 0x%02x%s", ret, newLine);
				}
				uart2BufferIndex = 0;
			}
		}
		if(flags & BUFFER_OVERRUN) {
			ret = console_handle_bufferOverrun();
			if(console_failureHandler(ret)) {
				printf_("CONSOLE: error handling buffer overrun: 0x%02x%s", ret, newLine);
			}
		}
	}
}

// ------------------------------------------------------------ PRIVATE

CONSOLE_RETVAL console_handle_rxByte() {
	HAL_StatusTypeDef ret;
	uint32_t flag;
	char c = (char)rx_byte;

	ret = HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY); // UART line echo
	if(halStatusHandler(ret)) {
		printf_("CONSOLE: echo transmit error: 0x%02x%s", ret, newLine);
	}

	if(c == '\r') {
		ret = HAL_UART_Transmit(&huart2, (uint8_t*)newLine, 2, HAL_MAX_DELAY);
		if(halStatusHandler(ret)) {
			printf_("CONSOLE: newline transmit error: 0x%02x%s", ret, newLine);
		}
		return CONSOLE_NEW_DATA;
	} else {
		uart2Buffer[uart2BufferIndex++] = c;
		if(uart2BufferIndex >= CONSOLE_UART_BUFFER_SIZE) {
			flag = osThreadFlagsSet(consoleThreadId, BUFFER_OVERRUN);
			if(flagErrorHandler(flag)) {
				printf_("CONSOLE: error setting flag: 0x%02x%s", ret, newLine);
			}
		}
	}
	return CONSOLE_OK;
}
CONSOLE_RETVAL console_handle_newInput() {
	osStatus_t ret;
	uint32_t flag_return;
	// Check semaphore if input is available
	ret = osSemaphoreAcquire(inputSem, MODULE_SEM_TIMEOUT);
	if(osStatusHandler(ret)) {
		return CONSOLE_FAILED_COPY;
	}
	// Request heap memory for data
	input = pvPortMalloc(sizeof(char)*uart2BufferIndex+1);
	if(input == NULL) {
		return CONSOLE_MEM_ERROR;
	}
	// Copy data
	memcpy(input, uart2Buffer, uart2BufferIndex);
	input[uart2BufferIndex] = '\0';
	// Release semaphore for input
	ret = osSemaphoreRelease(inputSem);
	if(osStatusHandler(ret)) {
		return CONSOLE_FAILED_SEMAPHORE;
	}
	// Notify parser about new message
	flag_return = osThreadFlagsSet(parserThreadId, NEW_INPUT);
	if(flagErrorHandler(flag_return)) {
		return CONSOLE_FAILED_FLAG;
	}
	return CONSOLE_OK;
}

CONSOLE_RETVAL console_handle_bufferOverrun() {
	console_clear_buffer();
	printf_("\r%s", uart2Buffer);
	printf_("CONSOLE: Max characters reached. Input was reset.%s", newLine);
	return CONSOLE_OK;
}

CONSOLE_RETVAL console_prep_peripherals() {
	HAL_StatusTypeDef ret = HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	if(halStatusHandler(ret)) {
		return CONSOLE_RESOURCE_ERROR;
	}
	return CONSOLE_OK;
}

CONSOLE_RETVAL console_clear_buffer() {
	memset(uart2Buffer, '\0', CONSOLE_UART_BUFFER_SIZE);
	uart2BufferIndex = 0;
	return CONSOLE_OK;
}

bool console_failureHandler(CONSOLE_RETVAL value) {
	switch(value) {
	case CONSOLE_OK:
		return false;
	case CONSOLE_NEW_DATA:
		return false;
	case CONSOLE_FAILED_COPY:
		printf_("CONSOLE: Failed copy!%s", newLine);
		break;
	case CONSOLE_FAILED_SEMAPHORE:
		printf_("CONSOLE: Failed semaphore!%s", newLine);
		break;
	case CONSOLE_FAILED_FLAG:
		printf_("CONSOLE: Failed flag!%s", newLine);
		break;
	case CONSOLE_RESOURCE_ERROR:
		printf_("CONSOLE: Resource error!%s", newLine);
		break;
	case CONSOLE_RW_ERROR:
		printf_("CONSOLE: Read/Write error!%s", newLine);
		break;
	case CONSOLE_ENABLE_ERROR:
		printf_("CONSOLE: Enable error!%s", newLine);
		break;
	case CONSOLE_INIT_ERROR:
		printf_("CONSOLE: Initialization error!%s", newLine);
		break;
	}
	return true;
}

