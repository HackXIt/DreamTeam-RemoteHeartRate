 /*
  * Example for Heart rate 4 click
  *
  * Date         Jan 2017
  * Author       Stefan Popovic
  * Copyright    GNU Public License v2
  *
  * Test configuration STM32 :
  *  MCU           :        STM32F107VC
  *  Dev. Board    :        EasyMx PRO v7 for STM32 ARM
  *  SW            :        mikroC PRO for ARM v4.9.0
  *
  *  Include example.pld file in your project.
  *  NOTES         :
  *                  - Place Heart rate 4 click board on mikroBUS slot 1
  *                  - Connect EasyTFT for additional information (turn on BCKL)
  *                  - Turn switches SW12.1 and SW12.2 in right position
  *                  - Use USB UART A
  *                  - IMPORTANT: If you cannot measure pulse with your finger
  *                     please, place your wrist on the sensor
  *                     ( radial pulse is located just under the thumb )
*******************************************************************************/
  #include "heartrate4_hw.h"
  #include "tft_resources.h"

  sbit HEART_RATE_4_INT at GPIOD_IDR.B10;

  char txt_val[ 18 ];
  char txt_milis[ 18 ];
  uint8_t sample_num = 0;
  bool read_f = false;
  bool stop_f = false;
  bool start_f = false;
  bool no_finger_f = true;
  static uint32_t miliseconds_counter = 0;
  uint32_t red_sample = 0;

  static void system_init( void )
  {

    // Enable digital output on PORTD
    GPIO_Digital_Output( &GPIOE_BASE, _GPIO_PINMASK_HIGH );
    GPIOE_ODR = 0xAAAA;

    // INT pin
    GPIO_Digital_Input( &GPIOD_BASE, _GPIO_PINMASK_10 );

    // I2C Initialization
    I2C1_Init_Advanced( 400000, &_GPIO_MODULE_I2C1_PB67 );

    // Initialize hardware UART1 module on PORTA
    UART1_Init_Advanced( 57600,
                         _UART_8_BIT_DATA,
                         _UART_NOPARITY,
                         _UART_ONE_STOPBIT,
                         &_GPIO_MODULE_USART1_PA9_10);

    Delay_ms( 100 );

    RCC_APB2ENR.AFIOEN = 1;              // Enable clock for alternate pin functions
    AFIO_EXTICR3 = 0x0300;               // PD10 as External interrupt
    EXTI_FTSR = 0x00000400;              // Set interrupt on Falling edge
    EXTI_IMR |= 0x00000400;              // Set mask
    NVIC_IntEnable(IVT_INT_EXTI15_10);   // Enable External interrupt
  }

  void InitTimer2(){
    RCC_APB1ENR.TIM2EN = 1;
    TIM2_CR1.CEN = 0;
    TIM2_PSC = 1;
    TIM2_ARR = 35999;
    NVIC_IntEnable(IVT_INT_TIM2);
    TIM2_DIER.UIE = 1;
    TIM2_CR1.CEN = 1;
  }

  void main( void )
  {
    system_init();
    display_init();
    hr4_init();
    hr4_set_registers();

    TFT_Set_Font( &HandelGothic_BT21x22_Regular, CL_RED, FO_HORIZONTAL );
    TFT_Write_Text( "Use MikroPlot Graph Generator", 10, 100 );
    TFT_Set_Font( &HandelGothic_BT21x22_Regular, CL_BLUE, FO_HORIZONTAL );
    TFT_Write_Text( "Place Finger On Sensor", 19, 170 );

    while ( true )
    {
      // Clearing the interrupt by reading the Interrupt Status 1
      hr4_is_new_fifo_data_ready();

      if ( read_f )                          // If INT was emitted
      {
         read_f = false;

         if ( !start_f )                     // First start
         {
            start_f = true;
            InitTimer2();                    // Initializing Timer 2
            EnableInterrupts();              // Enables the processor interrupt
            LOG("START\r\n");                // Sending START command to uPlot
         }

         red_sample = hr4_read_red();        // Read RED sensor data

         LongToStr(miliseconds_counter , txt_milis);
         LongToStr(red_sample , txt_val);
         Ltrim(txt_val);
         Ltrim(txt_milis);

         // If sample pulse amplitude is under threshold value ( proximity mode )
         if ( red_sample > 0 && red_sample < 32000 )
         {
           stop_f = true;
           if ( !no_finger_f )
           {
             TFT_Rectangle( 19, 170, 310, 200 );
             TFT_Write_Text( "Place Finger On Sensor", 19, 170 );
           }

           no_finger_f = true;
         }

         // If finger is detected ( we are in active heart rate mode )
         else if( red_sample != 0)
         {
             stop_f = false;
             
             if ( no_finger_f )
             {
              TFT_Rectangle( 19, 170, 310, 200 );
              TFT_Write_Text( "Generating Graph...", 19, 170 );
             }
             
             no_finger_f = false;
             
             // Sending data to MikroPlot in format:[pulse_value, milis] via UART
             LOG(txt_val);
             LOG(",");
             LOG(txt_milis);
             LOG("\r\n");
         }
      }

    }
  }

  void ExtInt() iv IVT_INT_EXTI15_10 ics ICS_AUTO {
    EXTI_PR.B10 = 1;            // clear flag
    read_f = true;
    }

  void Timer2_interrupt() iv IVT_INT_TIM2
  {
    TIM2_SR.UIF = 0;
    if (!stop_f)
    {
      miliseconds_counter++;
    }
  }
/************************************************************ END OF FILE *****/