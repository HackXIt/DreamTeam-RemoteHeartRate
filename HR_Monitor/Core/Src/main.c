/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "max30101.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
		.name = "defaultTask",
		.stack_size = 2000 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
MAX30101 hr_sensor;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */

	/**
	 * Initializes the sensor with the required configuration
	 * Sometime multiple approaches are required for a successfull
	 * sensor configuration - thus the while loop
	 */
	hr_sensor.i2c_handle = &hi2c1;
	while(MAX30101_initialize(&hr_sensor) != INIT_OK);
	char tmp_str[] = "Sensor configuration successfull!!!\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t *)tmp_str, strlen(tmp_str), 200);
	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 16;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Enable MSI Auto calibration
	 */
	HAL_RCCEx_EnableMSIPLLMode();
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00707CBB;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : LD3_Pin */
	GPIO_InitStruct.Pin = LD3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void putchar_(char character)
{
    // send char to console etc.
    HAL_UART_Transmit(&huart2, (uint8_t*)&character, 1, 100);
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
	/* USER CODE BEGIN 5 */

	/**
	 * Defines in which mode the data is analysed; either in real
	 * time or in a time series mode. If the real time mode is
	 * activated data samples are drawn from the memory and
	 * analysed on the go; the maximum analysed group of samples
	 * is defined via a parameter in the max30101.h file. The
	 * number of samples analysed has to take the delta threshold
	 * value into account defining at which point a new heart-beat
	 * analysis is started. In the time series mould a group of 200
	 * or more samples are analysed in a separate function. The number
	 * of samples penalised in the time service mode can be defined via
	 * a parameter in the max30101.h file.
	 */

	if(DATA_MODE == 0)
	{
		/**
		 * In RT-Mode only two samples are saved to calculate the delta of the
		 * current heart beat analysis.
		 */
		MAX30101_SAMPLE sample_old = { 0 };
		MAX30101_SAMPLE sample_new = { 0 };

		char teststr[50] = { 0 };

		int32_t delta_avg = 0;
		int32_t delta_sum = 0;
		uint64_t counter = 0;

		/**
		 * The RT analysis mode distinguishes between run and sample-values.
		 * Run value account for the analysis of 1 heart beat. Sample values
		 * are used to store the data of one sample group (usually 8). Low
		 * and high peaks are used to calculate the heart beat. The time between
		 * the low and high peak defines the speed of the heart beat.
		 */
		uint32_t run_peak_low_counter = 0;
		uint32_t run_peak_low_time= 0;
		uint32_t run_peak_low_value = 4000000;
		uint32_t sample_peak_low_value = 4000000;

		uint32_t run_peak_high_counter = 0;
		uint32_t run_peak_high_time = 0;
		uint32_t run_peak_high_value = 0;
		uint32_t sample_peak_high_value = 0;

		bool delta_neg_flag = false;
		//int32_t delta = 0;
		int16_t peak_to_peak_time = 0;

		/* Infinite loop */
		for(;;)
		{
			do
			{
				/**
				 * The sample peak values are reset at each run to make
				 * sure the lowest/highest point of the samples is found;
				 */
				sample_peak_low_value = 4000000;
				sample_peak_high_value = 0;
				delta_sum = 0;

				/**
				 * The MAX30101_get_interr_A_FULL checks if 8 samples (as defined
				 * in the MAX30101.h files are ready for being retrived from the
				 * FIFO memory of the chip.
				 */
				if(MAX30101_get_interr_A_FULL(&hr_sensor) == true)
				{
					/**
					 * A group of 8 samples (DELTA_SAMPLE) is retrived from the FIFO
					 */
					for(int i = 0; i < DELTA_SAMPLE; i++)
					{
						/**
						 * The MAX30101_read_sample function retrives a samples from the FIFO
						 */
						if(MAX30101_read_sample(&hr_sensor, &sample_new) == GET_REG_SUCCESS)
						{
							/**
							 * The if-counter function is a safeguard against the
							 * run-low value being reset after the decline in the
							 * red-led value after the peak. Otherwise the run-low
							 * time value might be reset to a value that is after
							 * the peak and thus turn the peak-to-peak time negative							 *
							 */

							if(counter < 50)
							{
								/**
								 * The following if statement ensure the lowest
								 * value of all samples in a run is set for the
								 * calculation of the peak-to-peak time
								 */

								if(run_peak_low_value > sample_new.red_led_value)
								{
									/**
									 * The following two statement set the low-runv value
									 * to the lowest point and the respective time as well
									 */
									run_peak_low_value = sample_new.red_led_value;
									run_peak_low_time = sample_new.tick_time;

									run_peak_low_counter = counter;
								}
							}

							/**
							 * The following if-statmetns set the run peak high
							 * value higher if a higher value than is encountered
							 * as in the previous group of samples
							 */

							if(run_peak_high_value < sample_new.red_led_value)
							{
								/**
								 * The run-high value is set higher as well as
								 * the run-high peak time to ensure a conformity
								 * witht the calculations
								 */
								run_peak_high_value = sample_new.red_led_value;
								run_peak_high_time = sample_new.tick_time;

								run_peak_high_counter = counter;
							}

							/**
							 * The following two if-statments set the samples high
							 * and low values in order to calculate the delta and
							 * thus the slop of the current sample. Afterwards the
							 * counter is increased used as a safeguard to not under
							 * mine the run low value when the values / delta is in
							 * a steep decline before a new run / heart beat starts.
							 */
							if(sample_peak_low_value > sample_new.red_led_value) sample_peak_low_value = sample_new.red_led_value;
							if(sample_peak_high_value < sample_new.red_led_value) sample_peak_high_value = sample_new.red_led_value;
							counter++;
						}

						/**
						 * The following if-statements calculates the sum of change over
						 * all values in this one group of samples (usually 8) in order
						 * to be able to calculate the average delta, i.e. if the curve
						 * is rising or in steep decline (i.e. a new heart beat starts).
						 * Afterwards the new_samples obtained is overwriting the old
						 * sample value used to calculate the delta between the samples
						 * taken i.e. the delta from the current and previous sample.
						 */
						if(i > 0) delta_sum = delta_sum + (sample_new.red_led_value - sample_old.red_led_value);
						sample_old = sample_new;

						//					sprintf(teststr, "RV:%8lu:  Time:%8lu\r\n", sample_new.red_led_value, sample_new.tick_time);
						//					HAL_UART_Transmit(&huart2, (uint8_t *)teststr, strlen(teststr), 200);
					}
				}


				/**
				 * Loop runs until sample_peak_low_value = is larger than idle values
				 * i.e. around 3000 at normal lighing
				 */
			} while(sample_peak_low_value < 5000 );


			/***
			 * The delta average is calculated to determine if the cureve is in
			 * a steep decline, i.e. marking the beginning of a new heart beat.
			 */
			delta_avg = delta_sum / DELTA_SAMPLE;

			/**
			 * If the delta average is below the threshold set in the MAX30101.h
			 * file, than the slope is consider to be in a steep decline, i.e. the
			 * end of a heart beat and the start of a new one. The DELTA_NEG_THRES
			 * needs to take into account the DELTA_SAMPLE size, i.e. the larger
			 * the sample the lower the DELTA_NEG_THRES since a larger sample
			 * will yield lower decline values.
			 */
			if(delta_avg < DELTA_NEG_THRES)
			{
				/***
				 * The delta negative flag (delta_neg_flag) indicates that until
				 * this sample the delta was positive, i.e. the new values belonged
				 * to the current heart beat signal.
				 */
				if(delta_neg_flag == false)
				{
					/***
					 * The peak_to_peak_time is calculated based on when the sample with
					 * the lowest red-led value has been retrieved from the FIFO and the
					 * time when the sample with the highest red-led value was retrieved
					 * from the FIFO. Due to the imprecise clock of the microcontroller
					 * a certain noise-deviation in the heart rate can't be avoided.
					 */
					peak_to_peak_time = run_peak_high_time - run_peak_low_time;

					/***
					 * The heart rate is calculated by dividing the peak-to-peak value
					 * by 1000 to obtain the ration of heart beats in a minute and
					 * muliply that value by 60. This formular has been found in an
					 * auxiliary document to the MAX30101 sensor - https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwjkz_rV2d7_AhXSRvEDHTLwCM4QFnoECA0QAQ&url=https%3A%2F%2Fpdfserv.maximintegrated.com%2Fen%2Fan%2FAN6409.pdf&usg=AOvVaw0jHhasO1DeNEJfxw1rJ34F&opi=89978449
					 */

					/***
					 * Two measures are used to calculate the heart rate.
					 * 1. The assigned to the sample when retrieved from the FIFO
					 * is used. The time of the low peak is deducted from the time
					 * of the high peak.
					 * 2. The time from low to high peak is calculated based on the
					 * distance of the two samples; i.e. high and low peak run counter.
					 * Afterwards the difference is multiplied with 10(ms) since
					 * 3200 samples are made every second and averaged by 32, i.e.
					 * the sensor produces 100 samples each second, i.e. 1000ms.
					 *
					 * When the time-distance between the peak has been obtained
					 * the value is used to devide 1000 ms and multiply that by
					 * 60 as indicated in the formular in the companion document
					 * to the sensor
					 *
					 */
					float heart_rate = (1000 / (float)peak_to_peak_time) * 60;
					float heart_rate_calc = ((1000 /((run_peak_high_counter - run_peak_low_counter) * 10) * 60));

					//sprintf(teststr, "::::::::::P2P High Time: %8ld\r\n", run_peak_high_time);
					//HAL_UART_Transmit(&huart2, (uint8_t *)teststr, strlen(teststr), 200);
					//sprintf(teststr, "::::::::::P2P Low Time: %8ld\r\n", run_peak_low_time);
					//HAL_UART_Transmit(&huart2, (uint8_t *)teststr, strlen(teststr), 200);
					//sprintf(teststr, "::::::::::P2P Time: %8d\r\n", peak_to_peak_time);
					//HAL_UART_Transmit(&huart2, (uint8_t *)teststr, strlen(teststr), 200);

					sprintf(teststr, "HEART RATE: %d\r\n", (uint8_t)heart_rate);
					HAL_UART_Transmit(&huart2, (uint8_t *)teststr, strlen(teststr), 200);
					sprintf(teststr, "HEART RATE CB: %d\r\n\r\n", (uint8_t)heart_rate_calc);
					HAL_UART_Transmit(&huart2, (uint8_t *)teststr, strlen(teststr), 200);

					uint8_t send_hr = (uint8_t)heart_rate_calc;
					HAL_StatusTypeDef status;

					status = HAL_I2C_Mem_Write(&hi2c1, (0x69 << 1), 0x00, sizeof(uint8_t), &send_hr, sizeof(uint8_t), 200);
					if(status != HAL_OK)
					{
						translate_I2C_Error(&hi2c1);
					}

					/**
					 * The delta_neg_flag is set to true to avoid another calculation of an
					 * heart rate after the peak value has been reached and the curve has
					 * been in a steep decline.
					 */
					run_peak_low_value = 4000000;
					run_peak_high_value = 0;

					run_peak_high_counter = 0;
					run_peak_low_counter = 0;
					counter = 0;

					delta_neg_flag = true;

				}
			}

			/***
			 * If the delta average turns positive again after a steep decline, i.e the
			 * start of a new heart beat the delta_neg_flag is set back to false;
			 */
			if(delta_avg > 0)
			{
				delta_neg_flag = false;
			}
		}
	}
	/* USER CODE END 5 */
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM16) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
