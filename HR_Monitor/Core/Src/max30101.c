
/*--- COMMON LIBRARIES ---*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
/*--- CUSTOM LIBRARIES ---*/
#include "max30101.h"
#include "stm32l4xx_hal.h"
#include "printf.h"
#include "stm32l4xx_ll_i2c.h" // for LowLevel driver


/*--- MACROS ---*/

/*--- Global variables ---*/

/* -------- INITIALIZATION -------- */

/***
 * translate_I2C_Error provides a detailed analysis of the I2C Error
 * and prints it to the terminal via the printf_ function (see printf_.c & printf_.h)
 * @param hi2c the I2C-Interface to be analyised
 */
void translate_I2C_Error(I2C_HandleTypeDef *hi2c) {
	uint32_t error_value = HAL_I2C_GetError(hi2c);
	printf_("I2C: ");
	// NOTE: Some errors need to be cleared by software
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

/***
 * The MAX30101_initialize function initializes the sensor with the
 * required values for taking measures
 * @param sensor The heart rate sensor to be intialized (i.e. it's hardware address)
 * @return Returns INIT_OK if the sensor configuration was successfull or CONFIG_FAILED otherwise
 */

INIT_STATUS MAX30101_initialize(MAX30101 *sensor)
{
	uint8_t tmp_config = 0;
	if(MAX30101_reset(sensor) != RES_SEN_SUCCESS) return CONFIG_FAILED;

	tmp_config = 0 | IE01_A_FULL_EN | IE01_PPG_RDY_EN | IE01_ALC_OVF_EN;
	if(MAX30101_set_interrupt_enabled01(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	tmp_config = 0 | DIE_TEMP_RDY;
	if(MAX30101_set_interrupt_enabled02(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	//tmp_config = 0 | FIFO_SMP_AVE_8 | FIFO_ROLLOVER_DIS | FIFO_A_FULL_16;
	tmp_config = 0 | FIFO_SMP_AVE_32 | FIFO_ROLLOVER_DIS | FIFO_A_FULL_24;
	if(MAX30101_set_FIFO_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	tmp_config = 0 | MODE_HR;
	//tmp_config = 0 | MODE_SPO2;
	//tmp_config = 0 | MODE_MULTI_LED;
	if(MAX30101_set_MODE_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	tmp_config = 0 | SPO2_ADC_RGE_00 | SPO2_SR_3200 | SPO2_LED_PW69;
	if(MAX30101_set_SPO2_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	tmp_config = 0;
	//if(MAX30101_set_LED1_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;
	if(MAX30101_set_LED2_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;
	if(MAX30101_set_LED3_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;
	if(MAX30101_set_LED4_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	tmp_config = 32;
	if(MAX30101_set_LED1_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;
	//if(MAX30101_set_LED2_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;
	//    tmp_config = 0;
	//    if(MAX30101_set_LED3_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;
	//    tmp_config = 0;
	//    if(MAX30101_set_LED4_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	tmp_config = 0;
	if(MAX30101_set_mulit_led_slot_0102_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;
	if(MAX30101_set_mulit_led_slot_0304_PA_config(sensor, &tmp_config) != SET_REG_SUCCESS) return CONFIG_FAILED;

	return INIT_OK;
}

/***
 * MAX30101_read_sample reads a single sample of values (currently only red-led values)
 * from the sensor's FIFO
 * @param sensor The hardware address of the sensor's FIFO the sample shall be retrieved from
 * @param sample A pointer to the MAX30101_SAMPLE struct the data can be stored in
 * @return Returns GET_REG_SUCCESS if a sample was succeessfully retrieved from the sensor's FIFO
 * return GET_REG_FAILED if reading the FIFO resulted in a failure;
 */
INIT_STATUS MAX30101_read_sample(MAX30101 *sensor, MAX30101_SAMPLE *sample)
{
	uint8_t reg_data[(ACTIVE_LED_CHANNELS * 3)] = { 0 };

	if(HAL_I2C_Mem_Read(sensor->i2c_handle, MAX30101_I2C_READ,  FIFO_DATA,  I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, sizeof(reg_data), I2C_TIMEOUT) != HAL_OK) return GET_REG_FAILED;
	else
	{
		uint32_t tmp_value = 0;
		if ( ACTIVE_LED_CHANNELS >= 1)
		{
			tmp_value |= (reg_data[0] << 16);
			tmp_value |= (reg_data[1] << 8);
			tmp_value |= (reg_data[2]);
			sample->red_led_value = tmp_value;
			sample->tick_time = HAL_GetTick();
		}

		sample->tick_time = HAL_GetTick();
	}
	return GET_REG_SUCCESS;
}

/***
 * MAX30101_reset resets the configuration of the sensor and all its values
 * @param sensor The HR-sensor to be reset
 * @return RES_SEN_SUCCESS if the reset was successful; RES_SEN_FAILED if sensor reset failed
 */
INIT_STATUS MAX30101_reset(MAX30101 *sensor)
{
	uint8_t tmp = 0;
	tmp = 0 | (1 << 6);
	if (MAX30101_set_MODE_config(sensor, &tmp) != SET_REG_SUCCESS) return RES_SEN_FAILED;
	return RES_SEN_SUCCESS;
}

/*---------------- GETTER & SETTER FOR ALL SENSOR REGISTERS ------------------- */
INIT_STATUS MAX30101_get_interrupt_enabled01(MAX30101 *sensor,uint8_t *reg_value)
{
	if(MAX30101_read_register(sensor, INTERRUPT_ENABLE01, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_interrupt_enabled01(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, INTERRUPT_ENABLE01, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_interrupt_status01(MAX30101 *sensor, uint8_t *reg_value)
{
	if(MAX30101_read_register(sensor, INTERRUPT_STATUS01, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_interrupt_status01(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, INTERRUPT_STATUS02, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}


INIT_STATUS MAX30101_get_interrupt_enabled02(MAX30101 *sensor,uint8_t *reg_value)
{
	if(MAX30101_read_register(sensor, INTERRUPT_ENABLE02, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_interrupt_enabled02(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, INTERRUPT_ENABLE02, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_interrupt_status02(MAX30101 *sensor, uint8_t *reg_value)
{
	if(MAX30101_read_register(sensor, INTERRUPT_STATUS02, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_interrupt_status02(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, INTERRUPT_STATUS02, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

/***
 * MAX30101_get_interr_A_FULL checks if the FIFO register is full (full defines as
 * in the MAX30101.h file, i.e. 32, 16, etc. samples in the FIFO
 * @param sensor HR-sensor to check the intrerrup for
 * @return Returns true when interrupt is set; false otherwise
 */
bool MAX30101_get_interr_A_FULL(MAX30101 *sensor)
{
	uint8_t buffer = 0;
	if(MAX30101_get_interrupt_status01(sensor, &buffer) != GET_REG_SUCCESS)
	{
		return false;
	} else
	{
		if (buffer & (1 << 7)) return true;
	}
	return false;
}

/***
 * MAX30101_get_interr_PPG_RDY checks if a new samples is read in the FIFO to be read
 * @param sensor The HR sensor to check the interrupt for
 * @return Returns true when a new sample is ready to be read; false otherwise
 */
bool MAX30101_get_interr_PPG_RDY(MAX30101 *sensor)
{
	uint8_t buffer = 0;

	if(HAL_I2C_Mem_Read(sensor->i2c_handle, MAX30101_I2C_READ, INTERRUPT_STATUS01, I2C_MEMADD_SIZE_8BIT, &buffer, sizeof(uint8_t), I2C_TIMEOUT) == HAL_OK)
	{
		if (buffer & (1 << 6))
		{
			return true;
		}
	}
	return false;
}

INIT_STATUS MAX30101_get_interr_DIE_TEMP_RDY(MAX30101 *sensor)
{
	uint8_t buffer = 0;
	if(MAX30101_get_interrupt_status02(sensor, &buffer) == GET_REG_SUCCESS)
	{
		if (buffer & (1 << 2)) return FLAG_SET;
	}
	return FLAG_NOT_SET;
}

/***
 * MAX30101_clear_FIFO clears the FIFO read/write pointer and overflow pointer
 * @param sensor The HR-Sensor the FIFO need to be cleared for
 * @return SET_REG_SUCCESS in case of a success
 */

INIT_STATUS MAX30101_clear_FIFO(MAX30101 *sensor)
{
	uint8_t tmp = 0;
	if (MAX30101_write_register(sensor, FIFO_WR_PTR, &tmp) != HAL_OK) return SET_REG_FAILED;
	if (MAX30101_write_register(sensor, FIFO_RD_PTR, &tmp) != HAL_OK) return SET_REG_FAILED;
	if (MAX30101_write_register(sensor, FIFO_OVF_COUNTER, &tmp) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_FIFO_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, FIFO_CONFIG, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_FIFO_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, FIFO_CONFIG, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_MODE_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, MODE_CONFIG, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_MODE_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, MODE_CONFIG, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_SPO2_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, SPO2_CONFIG, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_SPO2_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, SPO2_CONFIG, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_LED1_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, LED_PulseAmp_LED1_PA, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_LED1_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, LED_PulseAmp_LED1_PA, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_LED2_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, LED_PulseAmp_LED2_PA, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_LED2_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, LED_PulseAmp_LED2_PA, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_LED3_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, LED_PulseAmp_LED3_PA, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_LED3_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, LED_PulseAmp_LED3_PA, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_LED4_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, LED_PulseAmp_LED4_PA, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_LED4_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, LED_PulseAmp_LED4_PA, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_mulit_led_slot_0102_PA_config(MAX30101 *sensor, uint8_t *reg)
{
	if (MAX30101_read_register(sensor, MULTI_LED_SLOT01_AND_02, reg) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_mulit_led_slot_0102_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, MULTI_LED_SLOT01_AND_02, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_mulit_led_slot_0304_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, MULTI_LED_SLOT03_AND_04, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_mulit_led_slot_0304_PA_config(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_write_register(sensor, MULTI_LED_SLOT03_AND_04, reg_value) != HAL_OK) return SET_REG_FAILED;
	return SET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_temp_int(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, TEMP_INTEGER, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_get_temp_frac_int(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, TEMP_FRACTION, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

INIT_STATUS MAX30101_set_temp_en_int(MAX30101 *sensor, uint8_t *reg_value)
{
	if (MAX30101_read_register(sensor, TEMP_EN, reg_value) != HAL_OK) return GET_REG_FAILED;
	return GET_REG_SUCCESS;
}

/* -------- ABSTRACTION FUNCTIONS -------- */

/***
 * MAX30101_read_register reads one byte from a HR-sensor register
 * @param sensor The HR-Sensor to read the register from
 * @param reg The register of the HR-sensor to read the data from
 * @param data A pointer to the data buffer to read the data from the sensor into
 * @return returns the HAL Status
 */
HAL_StatusTypeDef MAX30101_read_register(MAX30101 *sensor, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(sensor->i2c_handle, MAX30101_I2C_READ, reg, I2C_MEMADD_SIZE_8BIT, data, I2C_MEMADD_SIZE_8BIT, I2C_TIMEOUT);
}

/***
 * MAX30101_MAX30101_read_registers reads multiple byte from a HR-sensor register
 * @param sensor The HR-Sensor to read the register from
 * @param reg The register of the HR-sensor to read the data from
 * @param data
 * @param size the size, i.e. number of registers to read from the HR sensor
 * @return returns the HAL Status
 */
HAL_StatusTypeDef MAX30101_read_registers(MAX30101 *sensor, uint8_t reg, uint8_t *data, uint8_t size)
{
	return HAL_I2C_Mem_Read(sensor->i2c_handle, MAX30101_I2C_READ, reg, I2C_MEMADD_SIZE_8BIT, data, size, I2C_TIMEOUT);
}

/***
 * MAX30101_write_register writes one byte to a register of choice of the HR sensor
 * @param sensor The HR-Sensor to write the register to
 * @param reg The register of the HR-sensor to write the data to
 * @param data A pointer to the data buffer that contains the data to be written to the sensor's register
 * @return returns the HAL Status
 */
HAL_StatusTypeDef MAX30101_write_register(MAX30101 *sensor, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(sensor->i2c_handle, MAX30101_I2C_WRITE, reg, I2C_MEMADD_SIZE_8BIT , data, I2C_MEMADD_SIZE_8BIT , I2C_TIMEOUT);
}
