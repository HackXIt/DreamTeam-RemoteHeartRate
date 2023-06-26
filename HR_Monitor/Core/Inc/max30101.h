#ifndef MAX30101_H
#define MAX30101_H

/* -------- IMPORTS -------- */
#include <stdbool.h>
#include "stm32l4xx_hal.h" /* Required for HAL_Library Functions */
#include "cmsis_os2.h"
/* -------- DEFINITIONS -------- */

extern UART_HandleTypeDef huart2;

/* NOTE Generic definitions for MAX30101 */
#define DATA_MODE 0 // 0 for near real-time analysis; 1 for time series analysis
#define MAX_READ_SIZE 64 // Total bytes of FIFO data according to P.13
#define MAX_SAMPLES 16// Total number of samples in memory according to P.13
#define CHANNEL_SIZE 3
#define SAMPLE_SIZE 3 // 3 bytes per sample
#define ACTIVE_LED_CHANNELS 1 // multiplied with SAMPLE_SIZE when data is read
#define LED_CURRENT 31 // Standard LED Current
#define MAX_RETRY 20 // Maximum amount of retries for I2C communication retries
#define HR_SAMPLE_SIZE 300
#define DELTA_SAMPLE 8 // Sample + 1 due to module calculations for counter
#define DELTA_NEG_THRES -25

#define INTERR_NON 0x00
#define INTERR_FLAG 0x01

#define HR_THRESHOLD 100000
/* NOTE I2C definitions for MAX30101 */

#define SLAVE_ID 0x57
#define MAX30101_I2C_WRITE 0xAE // This already takes the write-bit into account
#define MAX30101_I2C_READ 0xAF // This already takes the read-bit into account
#define SETUP_DELAY 20
#define I2C_TIMEOUT 100
#define UART_TIMEOUT 30
#define WRITE 0x00
#define READ 0x01

/* NOTE 8-Bit Registers of MAX30101 - Datasheet P.10 Table 1 */

#define INTERRUPT_STATUS01 0x00 // Interrupt status
#define INTERRUPT_STATUS02 0x01 // Interrupt status
#define INTERRUPT_ENABLE01 0x02 // Interrupt enable
#define INTERRUPT_ENABLE02 0x03 // Interrupt enable

#define FIFO_WR_PTR 0x04
#define FIFO_OVF_COUNTER 0x05
#define FIFO_RD_PTR 0x06
#define FIFO_DATA 0x07

#define FIFO_CONFIG 0x08 // FIFO configuration
#define FIFO_SMP_AVE_DIS 0x00 // 1 (no averaging)
#define FIFO_SMP_AVE_2 0x20 // 2  samples averaged per fifo sample
#define FIFO_SMP_AVE_4 0x40 // 4  samples averaged per fifo sample
#define FIFO_SMP_AVE_8 0x60 // 8  samples averaged per fifo sample
#define FIFO_SMP_AVE_16 0x80 // 16  samples averaged per fifo sample
#define FIFO_SMP_AVE_32 0xA0 // 32  samples averaged per fifo sample

#define FIFO_ROLLOVER_EN 0x10 //
#define FIFO_ROLLOVER_DIS 0x00 //
/***
 * Bit 4: FIFO Rolls on Full (FIFO_ROLLOVER_EN)
This bit controls the behavior of the FIFO when the FIFO becomes completely filled with data. If FIFO_ROLLOVER_EN
is set (1), the FIFO Address rolls over to zero and the FIFO continues to fill with new data. If the bit is not set (0), then the
FIFO is not updated until FIFO_DATA is read or the WRITE/READ pointer positions are changed.**/

#define FIFO_A_FULL_0 0x00 // 0 empty data samples - 32 unread data samples
#define FIFO_A_FULL_8 0x08 // 7 empty data samples - 25 unread data samples
#define FIFO_A_FULL_16 0x0F// 15 empty data samples - 17 unread data samples
#define FIFO_A_FULL_14 0x0E // 16 empty data samples
#define FIFO_A_FULL_24 0x08 // 23 empty data samples
/***
 * Bits 3:0: FIFO Almost Full Value (FIFO_A_FULL)
This register sets the number of data samples (3 bytes/sample) remaining in the FIFO when the interrupt is issued. For
example, if this field is set to 0x0, the interrupt is issued when there is 0 data samples remaining in the FIFO (all 32
FIFO words have unread data). Furthermore, if this field is set to 0xF, the interrupt is issued when 15 data samples are
remaining in the FIFO (17 FIFO data samples have unread data).
 */

#define MODE_CONFIG 0x09 // Mode configuration
#define MODE_SHDN 0x80
/***
 * Bit 7: Shutdown Control (SHDN)
The part can be put into a power-save mode by setting this bit to one. While in power-save mode, all registers retain their
values, and write/read operations function as normal. All interrupts are cleared to zero in this mode.
 */

#define MODE_RESET 0x40
/***
 * Bit 6: Reset Control (RESET)
When the RESET bit is set to one, all configuration, threshold, and data registers are reset to their power-on-state through
a power-on reset. The RESET bit is cleared automatically back to zero after the reset sequence is completed. Note:
Setting the RESET bit does not trigger a PWR_RDY interrupt event.
 */

#define MODE_HR 0x02 // Heart Rate Mode
#define MODE_SPO2 0x03 // Spo2 Mode
#define MODE_MULTI_LED 0x07 // Multi-LED Mode


#define SPO2_CONFIG 0x0A // SPO2 Configuration
/***
 * This register sets the SpO2 sensor ADC’s full-scale range as shown in Table 5.
 */
#define SPO2_ADC_RGE_00 0x00 // LSB Size (pA) 7.81; FULL SCALE (nA) 2048
#define SPO2_ADC_RGE_01 0x10 // LSB Size (pA) 15.63; FULL SCALE (nA) 4096
#define SPO2_ADC_RGE_02 0x20 // LSB Size (pA) 31.25; FULL SCALE (nA) 8192
#define SPO2_ADC_RGE_03 0x30 // LSB Size (pA) 62.5; FULL SCALE (nA) 16384

/***
 * Bits 4:2: SpO2 Sample Rate Control
These bits define the effective sampling rate with one sample consisting of one IR pulse/conversion, one RED pulse/
conversion, and one GREEN pulse/conversion. The sample rate and pulse-width are related in that the sample rate sets
an upper bound on the pulse-width time. If the user selects a sample rate that is too high for the selected LED_PW
setting, the highest possible sample rate is programmed instead into the register.
 */

#define SPO2_SR_50 0x00 // 50 samples per second
#define SPO2_SR_100 0x04 // 100 samples per second
#define SPO2_SR_200 0x08 // 200 samples per second
#define SPO2_SR_400 0x0C // 400 samples per second
#define SPO2_SR_800 0x10 // 800 samples per second
#define SPO2_SR_1000 0x11 // 1000 samples per second
#define SPO2_SR_1600 0x18 // 1600 samples per second
#define SPO2_SR_3200 0x1C // 3200 samples per second

/***
 * Bits 1:0: LED Pulse Width Control and ADC Resolution
These bits set the LED pulse width (the IR, Red, and Green have the same pulse width), and, therefore, indirectly sets
the integration time of the ADC in each sample. The ADC resolution is directly related to the integration time.
Table
 */

#define SPO2_LED_PW69 0x00 // PULSE WIDTH (µs) 69; ADC Resolution (bits) 15
#define SPO2_LED_PW118 0x01 // PULSE WIDTH (µs) 118; ADC Resolution (bits) 16
#define SPO2_LED_PW215 0x02 // PULSE WIDTH (µs) 215; ADC Resolution (bits) 17
#define SPO2_LED_PW411 0x03 // PULSE WIDTH (µs) 411; ADC Resolution (bits) 18


#define LED_PulseAmp_LED1_PA 0x0C // LED Configuration
#define LED_PulseAmp_LED2_PA 0x0D // LED Configuration
#define LED_PulseAmp_LED3_PA 0x0E // LED Configuration
#define LED_PulseAmp_LED4_PA 0x0F // LED Configuration

//Multi-LED Mode Control Registers Slot01 & Slot02
#define MULTI_LED_SLOT01_AND_02 0x11
#define MULTI_LED_SLOT03_AND_04 0x12

/***
 * Each slot generates a 3-byte output into the FIFO. One sample comprises all active slots, for example if SLOT1 and
SLOT2 are non-zero, then one sample is 2 x 3 = 6 bytes. If SLOT1 through SLOT3 are all non-zero, then one sample is 3
x 3 = 9 bytes. The slots should be enabled in order (i.e., SLOT1 should not be disabled if SLOT2 or SLOT3 are enabled).
*Both LED3 and LED4 are wired to Green LED. Green LED sinks current out of LED3_PA[7:0] and LED4_PA[7:0]
configurationin Multi-LED Mode and SLOTx[2:0] = 011.
 */

#define MULTI_LED_SLOT01_NONE 0x00
#define MULTI_LED_SLOT01_LED1_RED 0x01
#define MULTI_LED_SLOT01_LED2_IR 0x02
#define MULTI_LED_SLOT01_LED3_GREEN 0x03
#define MULTI_LED_SLOT01_LED4_GREEN 0x03

#define MULTI_LED_SLOT02_NONE 0x00
#define MULTI_LED_SLOT02_LED1_RED 0x10
#define MULTI_LED_SLOT02_LED2_IR 0x20
#define MULTI_LED_SLOT02_LED3_GREEN 0x30
#define MULTI_LED_SLOT02_LED4_GREEN 0x30

#define MULTI_LED_SLOT03_NONE 0x00
#define MULTI_LED_SLOT03_LED1_RED 0x01
#define MULTI_LED_SLOT03_LED2_IR 0x02
#define MULTI_LED_SLOT03_LED3_GREEN 0x03
#define MULTI_LED_SLOT03_LED4_GREEN 0x03

#define MULTI_LED_SLOT04_NONE 0x00
#define MULTI_LED_SLOT04_LED1_RED 0x10
#define MULTI_LED_SLOT04_LED2_IR 0x20
#define MULTI_LED_SLOT04_LED3_GREEN 0x30
#define MULTI_LED_SLOT04_LED4_GREEN 0x30

#define TEMP_INTEGER 0x1F
/***
 * Temperature Integer
The on-board temperature ADC output is split into two registers, one to store the integer temperature and one to store
the fraction. Both should be read when reading the temperature data, and the equation below shows how to add the two
registers together:
TMEASURED = TINTEGER + TFRACTION
This register stores the integer temperature data in 2’s complement format, where each bit corresponds to 1°C.

0x00 = 0
0x7E = +127

0x80 = -128
0xFF = -1
 */

#define TEMP_FRACTION 0x20
/***
 * Temperature Fraction
This register stores the fractional temperature data in increments of 0.0625°C. If this fractional temperature is paired with
a negative integer, it still adds as a positive fractional value (e.g., -128°C + 0.5°C = -127.5°C).
 */


#define DIE_TEMP_CONFIG 0x21
#define TEMP_EN 0x01
/***
 * Temperature Enable (TEMP_EN)
This is a self-clearing bit which, when set, initiates a single temperature reading from the temperature sensor. This bit
clears automatically back to zero at the conclusion of the temperature reading when the bit is set to one.
 */


/***
 * The interrupts are cleared whenever the interrupt status register is read, or when the register that triggered the interrupt
is read. For example, if the SpO2 sensor triggers an interrupt due to finishing a conversion, reading either the FIFO data
register or the interrupt register clears the interrupt pin (which returns to its normal HIGH state). This also clears all the
bits in the interrupt status register to zero.
 */


/***
 * IE abreviation for Interrupt Enabled
 */
#define IE01 0x02
#define IE01_A_FULL_EN 0x80
#define FIFO_FULL 0x80
/***
 * In SpO2 and HR modes, this interrupt triggers when the FIFO write pointer has a certain number of free spaces
remaining. The trigger number can be set by the FIFO_A_FULL[3:0] register. The interrupt is cleared by reading the
Interrupt Status 1 register (0x00).
 */

#define IE01_PPG_RDY_EN 0x40
#define PPG_RDY 0x40
/***
 * PPG_RDY: New FIFO Data Ready
In SpO2 and HR modes, this interrupt triggers when there is a new sample in the data FIFO. The interrupt is cleared by
reading the Interrupt Status 1 register (0x00), or by reading the FIFO_DATA register.
 */

#define IE01_ALC_OVF_EN 0x20
#define ALC_OVF 0x20
/***
 * ALC_OVF: Ambient Light Cancellation Overflow
This interrupt triggers when the ambient light cancellation function of the SpO2/HR photodiode has reached its maximum
limit, and therefore, ambient light is affecting the output of the ADC. The interrupt is cleared by reading the Interrupt
Status 1 register (0x00).
 */

#define PWR_RDY 0x00
/***
 * PWR_RDY: Power Ready Flag
On power-up or after a brownout condition, when the supply voltage VDD transitions from below the undervoltage lockout
(UVLO) voltage to above the UVLO voltage, a power-ready interrupt is triggered to signal that the module is powered-up
and ready to collect data.
 */

//Interrupt Enable02 - REG ADDR 0x03
#define IE02 0x03
#define IE02_DIE_TEMP_RDY_EN 0x03
//Interrupt Flag
#define DIE_TEMP_RDY 0x02
/***
 * DIE_TEMP_RDY: Internal Temperature Ready Flag
When an internal die temperature conversion is finished, this interrupt is triggered so the processor can read the
temperature data registers. The interrupt is cleared by reading either the Interrupt Status 2 register (0x01) or the TFRAC
register (0x20).
 */

/* -------- STRUCTURES -------- */

/* NOTE Sensor Datastructure holding information */

typedef struct MAX30101{
    I2C_HandleTypeDef *i2c_handle; // Handle for I2C communication
    uint16_t RED_data[MAX_SAMPLES]; // All red led samples
} MAX30101;

typedef struct MAX30101_SAMPLE{
	uint32_t red_led_value;
	uint32_t tick_time;
} MAX30101_SAMPLE;

/* NOTE Error codes which may happen during initialization */
typedef enum {
	INIT_OK = 0x00,
    REVISION_FAILED,
    REVISION_FALSE,
    RESET_FAILED,
    CONFIG_FAILED,
    SPO2_CONFIG_FAILED,
    INITIAL_TEMP_FAILED,
    LED_CONFIG_FAILED,

    RES_SEN_FAILED,
	RES_SEN_SUCCESS = 0x0B,

	GET_REG_FAILED,
	GET_REG_SUCCESS,

	SET_REG_FAILED,
	SET_REG_SUCCESS,

	FLAG_SET,
	FLAG_NOT_SET,

	FIFO_READ_SUCCESS,
	FIFO_READ_FAILURE,

	READ_SAMPLE_SET_SUCCESS,
	READ_SAMPLE_SET_FAILURE,

	CALIBRATION_SUCCESS,
	CALIBRATION_FAILURE
} INIT_STATUS;

/* -------- INITIALIZATION -------- */

/************************************************
 * @brief Initializes the MAX30101 sensor and prepares sensor datastructure
 * @note The initialization is dependant on MACROS
 * @param sensor - Sensor datastructure
 * @param i2c_handle - I2C Communication handle to use & store in datastructure
 * @return INIT_STATUS - Returns initialization status code
 ***********************************************/
INIT_STATUS MAX30101_initialize(MAX30101 *sensor);

/* -------- ERROR HANDLNG -------- */
/***
 * translate_I2C_Error provides a detailed analysis of the I2C Error
 * and prints it to the terminal via the printf_ function (see printf_.c & printf_.h)
 * @param hi2c the I2C-Interface to be analyised
 */
void translate_I2C_Error(I2C_HandleTypeDef *hi2c);
/* -------- GETTER & SETTER FUNCTIONS FOR HR SENSOR REGISTERS -------- */

INIT_STATUS MAX30101_read_sample(MAX30101 *sensor, MAX30101_SAMPLE *sample);
INIT_STATUS MAX30101_reset(MAX30101 *sensor);
INIT_STATUS MAX30101_clear_FIFO(MAX30101 *sensor);

INIT_STATUS MAX30101_get_interrupt_enabled01(MAX30101 *sensor,uint8_t *reg_value);
INIT_STATUS MAX30101_set_interrupt_enabled01(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_get_interrupt_status01(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_interrupt_status01(MAX30101 *sensor, uint8_t *reg_value);
bool MAX30101_get_interr_PPG_RDY(MAX30101 *sensor);
bool MAX30101_get_interr_A_FULL(MAX30101 *sensor);

INIT_STATUS MAX30101_get_interrupt_enabled02(MAX30101 *sensor,uint8_t *reg_value);
INIT_STATUS MAX30101_set_interrupt_enabled02(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_get_interrupt_status02(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_interrupt_status02(MAX30101 *sensor, uint8_t *reg_value);

INIT_STATUS MAX30101_clear_FIFO(MAX30101 *sensor);
INIT_STATUS MAX30101_get_FIFO_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_FIFO_config(MAX30101 *sensor, uint8_t *reg_value);

INIT_STATUS MAX30101_get_MODE_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_MODE_config(MAX30101 *sensor, uint8_t *reg_value);

INIT_STATUS MAX30101_get_SPO2_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_SPO2_config(MAX30101 *sensor, uint8_t *reg_value);

INIT_STATUS MAX30101_set_LED1_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_get_LED1_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_LED2_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_get_LED2_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_LED3_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_get_LED3_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_LED4_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_get_LED4_PA_config(MAX30101 *sensor, uint8_t *reg_value);

INIT_STATUS MAX30101_get_mulit_led_slot_0102_PA_config(MAX30101 *sensor, uint8_t *reg);
INIT_STATUS MAX30101_set_mulit_led_slot_0102_PA_config(MAX30101 *sensor, uint8_t *reg);

INIT_STATUS MAX30101_get_mulit_led_slot_0304_PA_config(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_mulit_led_slot_0304_PA_config(MAX30101 *sensor, uint8_t *reg_value);

INIT_STATUS MAX30101_get_temp_int(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_get_temp_frac_int(MAX30101 *sensor, uint8_t *reg_value);
INIT_STATUS MAX30101_set_temp_en_int(MAX30101 *sensor, uint8_t *reg_value);

/* -------- ABSTRACTION FUNCTIONS -------- */

/************************************************
 * @brief Communication function, which reads 1 byte of data from given register of MAX30101
 *
 * @param sensor - Sensor datastructure
 * @param reg - Register of MAX30101 to be read
 * @param data - Pointer to target
 * @return HAL_StatusTypeDef - Returns communication status
 ***********************************************/
HAL_StatusTypeDef MAX30101_read_register(MAX30101 *sensor, uint8_t reg, uint8_t *data);
/************************************************
 * @brief Communication function, which reads X byte of data from given register of MAX30101
 * @note The only register where this is applicable is the FIFO, all others are only 1 byte
 * @param sensor - Sensor datastructure
 * @param reg - Register of MAX30101 to be read
 * @param data - Pointer to target (should have appropriate size)
 * @param size - Amount of bytes to be read
 * @return HAL_StatusTypeDef - Returns communication status
 ***********************************************/
HAL_StatusTypeDef MAX30101_read_registers(MAX30101 *sensor, uint8_t reg, uint8_t *data, uint8_t size);
/************************************************
 * @brief Communication function, which writes 1 byte of data into given register of MAX30101
 *
 * @param sensor - Sensor datastructure
 * @param reg - Register of MAX30101 to be written to
 * @param data - Pointer of source
 * @return HAL_StatusTypeDef - Returns communication status
 ***********************************************/
HAL_StatusTypeDef MAX30101_write_register(MAX30101 *sensor, uint8_t reg, uint8_t *data);
#endif
