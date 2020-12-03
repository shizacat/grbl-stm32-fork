/*
  main.c - An embedded CNC Controller with rs274/ngc (g-code) support
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "grbl.h"
// Declare system global variable structure
system_t sys;
int32_t sys_position[N_AXIS];      // Real-time machine (aka home) position vector in steps.
int32_t sys_probe_position[N_AXIS]; // Last probe position in machine coordinates and steps.
volatile uint8_t sys_probe_state;   // Probing state value.  Used to coordinate the probing cycle with stepper ISR.
volatile uint8_t sys_rt_exec_state;   // Global realtime executor bitflag variable for state management. See EXEC bitmasks.
volatile uint8_t sys_rt_exec_alarm;   // Global realtime executor bitflag variable for setting various alarms.
volatile uint8_t sys_rt_exec_motion_override; // Global realtime executor bitflag variable for motion-based overrides.
volatile uint8_t sys_rt_exec_accessory_override; // Global realtime executor bitflag variable for spindle/coolant overrides.
#ifdef DEBUG
  volatile uint8_t sys_rt_exec_debug;
#endif

#ifdef STM32F103C8
  #include "main.h"
  // #include "usb_lib.h"
  // #include "hw_config.h"
  // #include "stm32eeprom.h"
  
  #ifdef USEUSB
    #include "usb_device.h"
    // #include "usb_desc.h"
    // #include "usb_pwr.h"
  #else
    #include "stm32f10x_usart.h"
    void USART1_Configuration(u32 BaudRate)
    {
      GPIO_InitTypeDef GPIO_InitStructure;
      USART_InitTypeDef USART_InitStructure;
      NVIC_InitTypeDef NVIC_InitStructure;
      NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
      NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;   
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  

      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
      NVIC_Init(&NVIC_InitStructure);                 
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
      GPIO_Init(GPIOA, &GPIO_InitStructure);
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
      GPIO_Init(GPIOA, &GPIO_InitStructure);

      USART_InitStructure.USART_BaudRate = BaudRate;	  
      USART_InitStructure.USART_WordLength = USART_WordLength_8b; 
      USART_InitStructure.USART_StopBits = USART_StopBits_1;	 
      USART_InitStructure.USART_Parity = USART_Parity_No;	 
      USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
      USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
      USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE);
      USART_Init(USART1, &USART_InitStructure);
      //	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
      USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
      USART_Cmd(USART1, ENABLE);
    }
  #endif

  void SystemClock_Config(void);
#endif


#ifdef WIN32
  int main(int argc, char *argv[])
#else
  int main(void)
#endif
{
  #if defined (STM32F103C8)
    HAL_Init();

     /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    
    SystemClock_Config();

    /* SWD Enable */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
    
    #ifdef LEDBLINK
      GPIO_InitTypeDef GPIO_InitStructure;
      __HAL_RCC_GPIOC_CLK_ENABLE();
      GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
      GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStructure.Pin = GPIO_PIN_13;
      GPIO_InitStructure.Pull = GPIO_NOPULL; // ???
      HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
    #endif

    #ifdef USEUSB
      MX_USB_DEVICE_Init();
    #else
      USART1_Configuration(115200);
    #endif

    #ifndef NOEEPROMSUPPORT
      HAL_FLASH_Unlock();
      eeprom_init();
    #endif

  #endif

  // Initialize system upon power-up.
  serial_init();   // Setup serial baud rate and interrupts
  #ifdef WIN32
    winserial_init(argv[1]);
    eeprom_init();
  #endif

  settings_init(); // Load Grbl settings from EEPROM
  stepper_init();  // Configure stepper pins and interrupt timers
  system_init();   // Configure pinout pins and pin-change interrupt

  memset(sys_position,0,sizeof(sys_position)); // Clear machine position.
  #ifdef AVRTARGET
    sei(); // Enable interrupts
  #endif
  
  // Initialize system state.
  #ifdef FORCE_INITIALIZATION_ALARM
    // Force Grbl into an ALARM state upon a power-cycle or hard reset.
    sys.state = STATE_ALARM;
  #else
    sys.state = STATE_IDLE;
  #endif
  
  // Check for power-up and set system alarm if homing is enabled to force homing cycle
  // by setting Grbl's alarm state. Alarm locks out all g-code commands, including the
  // startup scripts, but allows access to settings and internal commands. Only a homing
  // cycle '$H' or kill alarm locks '$X' will disable the alarm.
  // NOTE: The startup script will run after successful completion of the homing cycle, but
  // not after disabling the alarm locks. Prevents motion startup blocks from crashing into
  // things uncontrollably. Very bad.
  #ifdef HOMING_INIT_LOCK
    if (bit_istrue(settings.flags,BITFLAG_HOMING_ENABLE)) { sys.state = STATE_ALARM; }
  #endif

  // Grbl initialization loop upon power-up or a system abort. For the latter, all processes
  // will return to this loop to be cleanly re-initialized.
  for(;;) {

    // Reset system variables.
    uint8_t prior_state = sys.state;
    memset(&sys, 0, sizeof(system_t)); // Clear system struct variable.
    sys.state = prior_state;
    sys.f_override = DEFAULT_FEED_OVERRIDE;  // Set to 100%
    sys.r_override = DEFAULT_RAPID_OVERRIDE; // Set to 100%
    sys.spindle_speed_ovr = DEFAULT_SPINDLE_SPEED_OVERRIDE; // Set to 100%
		memset(sys_probe_position,0,sizeof(sys_probe_position)); // Clear probe position.
    sys_probe_state = 0;
    sys_rt_exec_state = 0;
    sys_rt_exec_alarm = 0;
    sys_rt_exec_motion_override = 0;
    sys_rt_exec_accessory_override = 0;

    // Reset Grbl primary systems.
    serial_reset_read_buffer(); // Clear serial read buffer
    gc_init(); // Set g-code parser to default state
    spindle_init();
    coolant_init();
    limits_init();
    probe_init();
    plan_reset(); // Clear block buffer and planner variables
    st_reset(); // Clear stepper subsystem variables.

    // Sync cleared gcode and planner positions to current system position.
    plan_sync_position();
    gc_sync_position();

    // Print welcome message. Indicates an initialization has occured at power-up or with a reset.
    report_init_message();

    // Start Grbl main loop. Processes program inputs and executes them.
    protocol_main_loop();

  }
  return 0;   /* Never reached */
}

#if defined (STM32F103C8)
  /**
  * @brief System Clock Configuration
  * @retval None
  */
  void SystemClock_Config(void)
  {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
      Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }
  }
  void _delay_ms(uint32_t x)
  {
    uint32_t temp;
    SysTick->LOAD = (uint32_t)72000000 / 8000 * x;                 // Loading time
    SysTick->VAL = 0x00;                                      // Empty the counter
    SysTick->CTRL = 0x01;                                     // Start from bottom
    do
    {
      temp = SysTick->CTRL;
    } while (temp & 0x01 && !(temp&(1 << 16)));               // Wait time arrive
    SysTick->CTRL = 0x00;                                     // Close the counter
    SysTick->VAL = 0X00;                                      // Empty the counter
  }
  void LedBlink(void)
  {
    static GPIO_PinState nOnFlag = GPIO_PIN_SET;
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, nOnFlag);
    nOnFlag = (nOnFlag == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
  }
  /**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
  void Error_Handler(void)
  {
  }

  // extern PCD_HandleTypeDef hpcd_USB_FS;
  // void USB_LP_CAN1_RX0_IRQHandler(void){
  //   HAL_PCD_IRQHandler(&hpcd_USB_FS);
  // }

  // void SysTick_Handler(void)
  // {
  //   /* USER CODE BEGIN SysTick_IRQn 0 */

  //   /* USER CODE END SysTick_IRQn 0 */
  //   HAL_IncTick();
  //   /* USER CODE BEGIN SysTick_IRQn 1 */

  //   /* USER CODE END SysTick_IRQn 1 */
  // }

  void HAL_MspInit(void)
  {
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init*/

    /** ENABLE: JTAG-DP Disabled and SW-DP Disabled
    */
    

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
  }
#endif