/*
 * heartrateReceiver.c
 *
 *  Created on: Jun 23, 2023
 *      Author: rini
 */

#include "heartrateReceiver.h"

/* NOTE: I could not get DMA mode to work, it just sucks
 * The way I understand it, DMA mode is supposed to handle acknowledge appropriately by itself
 * But the corresponding interrupt routines never get called, even though they are for interrupt and DMA mode
 * I can trigger my events in the IRQ handler, but the sender never gets an acknowledge back.
 * I just ended up using interrupt mode because it just works.
 */
#define I2C_USING_INTERRUPTS

/* NOTE: When programming alone, I needed to simulate data coming in.
 * This macro MUST BE commented out on final project, since it breaks the application in production.
 * The macro activates a timer interrupt, which periodically sends I2C data over from I2C3 to I2C1.
 * Of course, the pins must be wired appropriately. For me it worked great to simulate data and verify my code.
 * WARNING: Be aware, that data simulation ONLY WORKS FOR THIS MODULE.
 * Due to the usage of USART1 TX/RX Pins for I2C1, the data simulation doesn't work in conjunction with other modules.
 */
//#define I2C_DATA_SIMULATION

/* NOTE: This macro activates more verbose logging information on USART2
 * Be aware, that due to the amount of logs transmitted, the performance of the application is affected badly.
 * It is however very helpful for viewing this module's behaviour.
 */
//#define HRRECEIVER_DEBUG

/* NOTE: This macro activates logging output upon an I2C_EVENT on USART2
 * When this macro is activated, a change in the heartrate receiver RAM is printed on USART2.
 * It is a secondary debug macro, which isn't necessary in production.
 */
//#define HRRECEIVER_OUTPUT_I2C_EVENT

// ------------------------------------------------------------ STATIC variables
// (configured in application.c)
extern const char *newLine;
extern osThreadId_t hrReceiverThreadId;
extern osThreadAttr_t hrReceiver_attr;
extern osSemaphoreId_t i2cSem;
extern uint8_t heartrate_ram[HEARTRATE_RAM_SIZE];
extern uint8_t heartrate_rx_offset;
osTimerId_t selfSendTimer;
bool first = true;
uint8_t newValue = 0;
uint8_t selfAddress_write = (0x69 << 1);
uint8_t selfAddress_read = (0x69 << 1) | 1;

// ------------------------------------------------------------ Interrupt routines

#ifdef I2C_USING_INTERRUPTS
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
	HAL_StatusTypeDef hal_return;
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: AddrCallback %s%s", TransferDirection==I2C_DIRECTION_RECEIVE ? "RX" : "TX", newLine);
#endif
	hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, &heartrate_rx_offset, I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
	if(halStatusHandler(hal_return)) {
		printf_("HRRECEIVER: error in address callback: ");
		translate_I2C_Error(hi2c);
	}
	/*
	if(first) {
		hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, &heartrate_rx_offset, I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
	} else {
		hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, &heartrate_ram[heartrate_rx_offset], I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
	}
	if(halStatusHandler(hal_return)) {
		printf_("HRRECEIVER: error in address callback: ");
		translate_I2C_Error(hi2c);
	}

	if( TransferDirection==I2C_DIRECTION_TRANSMIT ) {
		if( first ) {
			hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, &heartrate_rx_offset, I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
		} else {
			hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, &heartrate_ram[heartrate_rx_offset], I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
		}
	} else {
		hal_return = HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &heartrate_ram[heartrate_rx_offset], I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
	}
	if(halStatusHandler(hal_return)) {
		printf_("HRRECEIVER: eellurror in address callback: ");
		translate_I2C_Error(hi2c);
	}
	*/
}
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: ListenCpltCallback (0x%08lx)%s", hi2c->Devaddress, newLine);
#endif
	HAL_StatusTypeDef hal_return = HAL_I2C_EnableListen_IT(hi2c); // slave is ready again
	if(halStatusHandler(hal_return)) {
		printf_("ISR: error in interrupt peripherals: ");
		translate_I2C_Error(hi2c);
	}
	/*
	first = true;
	HAL_StatusTypeDef hal_return = HAL_I2C_EnableListen_IT(hi2c); // slave is ready again
	if(halStatusHandler(hal_return)) {
		printf_("ISR: error in interrupt peripherals: ");
		translate_I2C_Error(hi2c);
	}
	*/
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: SlaveRxCpltCallback%s", newLine);
#endif
	if(first) {
		first = false;
		HAL_StatusTypeDef hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, &heartrate_ram[heartrate_rx_offset], I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
		if(halStatusHandler(hal_return)) {
			printf_("ISR: error in interrupt peripherals: ");
			translate_I2C_Error(hi2c);
		}
	} else {
		first = true;
		// Notify task about idle event
		uint32_t ret = osThreadFlagsSet(hrReceiverThreadId, I2C_EVENT);
		if(flagErrorHandler(ret)) {
			printf_("ISR: thread flag error: 0x%08lx%s", ret, newLine);
		}
	}
	/*
	HAL_StatusTypeDef hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, heartrate_ram, I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
	if(halStatusHandler(hal_return)) {
		printf_("ISR: error in interrupt peripherals: ");
		translate_I2C_Error(hi2c);
	}
	*/
	/*
	if(hi2c->Instance == I2C3) {
	  // Notify task about idle event
	  uint32_t ret = osThreadFlagsSet(hrReceiverThreadId, I2C_EVENT);
	  if(flagErrorHandler(ret)) {
		  printf_("ISR: thread flag error: 0x%08lx%s", ret, newLine);
	  }
	}
	if(first) {
#ifdef HRRECEIVE_DEBUG
		printf_("RXCB: offset <== %3d%s", heartrate_rx_offset, newLine );
#endif
		first = false;
	} else {
#ifdef HRRECEIVE_DEBUG
		printf_("RXCB: ram[%3d] <== %3d%s", heartrate_rx_offset,  heartrate_ram[heartrate_rx_offset], newLine );
#endif
		heartrate_rx_offset++;
		if(heartrate_rx_offset >= HEARTRATE_RAM_SIZE) {
			first = true;
			heartrate_rx_offset = 0;
		}
	}
	HAL_StatusTypeDef hal_return = HAL_I2C_Slave_Seq_Receive_IT(hi2c, &heartrate_ram[heartrate_rx_offset], I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
	if(halStatusHandler(hal_return)) {
		printf_("ISR: error in slave RX complete callback: ");
		translate_I2C_Error(hi2c);
	}
	*/
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: SlaveTxCpltCallback%s", newLine );
#endif
	/*
	heartrate_rx_offset++;
	if(heartrate_rx_offset >= HEARTRATE_RAM_SIZE) {
		first = true;
		heartrate_rx_offset = 0;
	}
	HAL_StatusTypeDef hal_return = HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &heartrate_ram[heartrate_rx_offset], I2C_MEMADD_SIZE_8BIT, I2C_NEXT_FRAME);
	if(halStatusHandler(hal_return)) {
		printf_("ISR: error in slave TX complete callback: ");
		translate_I2C_Error(hi2c);
	}
	*/
}
#endif

#ifndef I2C_USING_INTERRUPTS
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: HAL_I2C_AddrCallback%s", newLine);
#endif
}
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: HAL_I2C_ListenCpltCallback%s", newLine);
#endif
}
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: HAL_I2C_SlaveTxCpltCallback%s", newLine);
#endif
}
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
#ifdef HRRECEIVER_DEBUG
	printf_("ISR: HAL_I2C_SlaveRxCpltCallback%s", newLine);
#endif
}
#endif

#ifdef I2C_DATA_SIMULATION
void sendDataSelf(void *argument) {
	HAL_StatusTypeDef hal_return;
	hal_return = HAL_I2C_Mem_Write(&hi2c1, selfAddress_write, heartrate_rx_offset, I2C_MEMADD_SIZE_8BIT, &newValue, I2C_MEMADD_SIZE_8BIT, HAL_MAX_DELAY);
	if(halStatusHandler(hal_return)) {
		printf_("ISR: sendDataSelf error: ");
		translate_I2C_Error(&hi2c1);
	}
	newValue++;
	if(newValue >= 255) {
		newValue = 0;
	}
}
#endif

// ------------------------------------------------------------ TASKS
// Tasks that run in FreeRTOS
void StartHrReceiver(void *argument) {
	hrReceiver_module_t *module = (hrReceiver_module_t*)argument;
	HRRECEIVER_RETVAL receiver_return = 0;
	hrReceiverThreadId = osThreadNew(HrReceiver, module, &hrReceiver_attr);
	receiver_return = hrReceiver_prep_peripherals();
	if(hrReceiver_failureHandler(receiver_return)) {
		// TODO handle initialization error in heartrateReceiver
	}
#ifdef I2C_DATA_SIMULATION
	selfSendTimer = osTimerNew(sendDataSelf, osTimerPeriodic, &newValue, NULL);
	if(osStatusHandler(osTimerStart(selfSendTimer, 10000))) {
		printf_("HRRECEIVER: Timer error!%s", newLine);
	}
#endif
	osThreadExit();
}
void HrReceiver(void *argument) {
	uint32_t flags_return;
	HRRECEIVER_RETVAL receiver_return;
	while(1) {
		flags_return = osThreadFlagsWait(I2C_EVENT, osFlagsWaitAny, osWaitForever);
		if(flagErrorHandler(flags_return)) {
			// On flag error, continue loop from beginning
			continue;
		}
		if(flags_return & I2C_EVENT) {
			receiver_return = hrReceiver_handle_heartrate();
			if(hrReceiver_failureHandler(receiver_return)) {
				printf_("HRRECEIVER: Error handling I2C event: 0x%02x%s", receiver_return, newLine);
			}
		}
	}
}

// ------------------------------------------------------------ PRIVATE
// Private functions used in tasks
HRRECEIVER_RETVAL hrReceiver_handle_heartrate() {
	//release_semaphore(i2cSem);
#ifdef HRRECEIVER_OUTPUT_I2C_EVENT
	printf_("HRRECEIVER: I2C Event:%s", newLine);
	for(uint8_t i = 0; i < HEARTRATE_RAM_SIZE; i++) {
		printf_("[%hhu]:%hhu\t", i, heartrate_ram[i]);
	}
	printf_("%s", newLine);
#endif
	return HRRECEIVER_OK;
}

void hrReceiver_i2c_callback() {
	printf_("HRRECEIVER: I2C callback%s", newLine);
}

HRRECEIVER_RETVAL hrReceiver_prep_peripherals() {
	HAL_StatusTypeDef hal_return;

#ifdef I2C_USING_INTERRUPTS
	hal_return = HAL_I2C_EnableListen_IT(&hi2c3);
	if(halStatusHandler(hal_return)) {
		printf_("HRRECEIVER: error in interrupt peripherals: ");
		translate_I2C_Error(&hi2c3);
		return HRRECEIVER_RESOURCE_ERROR;
	}
#endif

#ifndef I2C_USING_INTERRUPTS
	hal_return = HAL_I2C_EnableListen(&hi2c1);
	if(halStatusHandler(hal_return)) {
		printf_("HRRECEIVER: error in interrupt peripherals: ");
		translate_I2C_Error(&hi2c1);
		return HRRECEIVER_RESOURCE_ERROR;
	}
	// Receive 1 byte in DMA
	hal_return = HAL_I2C_Slave_Receive_DMA(&hi2c1, heartrate_ram, I2C_MEMADD_SIZE_8BIT);
	if(halStatusHandler(hal_return)) {
		printf_("HRRECEIVER: error in DMA peripherals: ");
		translate_I2C_Error(&hi2c1);
		return HRRECEIVER_RESOURCE_ERROR;
	}
#endif
	return HRRECEIVER_OK;
}

bool hrReceiver_failureHandler(HRRECEIVER_RETVAL value) {
	switch(value) {
	case HRRECEIVER_OK:
		return false;
	case HRRECEIVER_FAILED:
		printf_("HRRECEIVER: Failure!%s", newLine);
	case HRRECEIVER_RESOURCE_ERROR:
		printf_("HRRECEIVER: Resource error!%s", newLine);
	case HRRECEIVER_INIT_ERROR:
		printf_("HRRECEIVER: Initialization error!%s", newLine);
	}
	return true;
}
