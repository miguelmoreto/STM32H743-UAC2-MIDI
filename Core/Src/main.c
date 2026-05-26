/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include "tusb.h"
#include "midi.h"
#include "audio_cb.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUDIO_BUFFER_SIZE 	48//CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
  BLINK_MIDI_CC = 50
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;



uint8_t midi_rx_flag = 0;

MIDI_MsgUnion_typedef midi_rx_msg;





// Audio test data (the buffer should be located in a portion of a RAM
// that the USB device can access (check linker script).
__attribute__((section(".usb_ram")))
__attribute__((aligned(4)))
uint32_t test_buffer_audio[AUDIO_BUFFER_SIZE];
uint8_t usb_buffer[AUDIO_BUFFER_SIZE * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX];

volatile uint32_t system_ticks = 0;

#include <math.h>

#define FREQ        1000
#define AMPLITUDE   0.5

static float phase = 0.0f;
static float phase_inc = 2.0f * 3.1415926f * FREQ / CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
void MPU_Config_USB_D2_SRAM(void);
void led_blinking_task(void);
void audio_task(void);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  MPU_Config_USB_D2_SRAM();
  SCB_DisableDCache();
  SCB_DisableICache();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //Init device stack on configured roothub port:
  tusb_rhport_init_t dev_init = {
      .role = TUSB_ROLE_DEVICE,
      .speed = TUSB_SPEED_FULL};
  SCB_CleanInvalidateDCache();
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

  // Init values for UAC2 example callbacks:
  audio_init();

  printf("Moreto STM32H743 TinyUSB Audio and MIDI example\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  tud_task();// tinyusb device task
	  audio_task();
	  led_blinking_task();

	  if (midi_rx_flag){ // MIDI message received
		  // Read the 4 byte packet and store it in the union array:
		  tud_midi_n_packet_read(0, midi_rx_msg.array);
		  printf("Midi message. CIN: %#04x Cable: %#04x Channel: %#04x Type: %#04x  "
				  "Data1: %#04x  Data2: %#04x\r\n", midi_rx_msg.fields.CIN, midi_rx_msg.fields.cable,
				  midi_rx_msg.fields.channel, midi_rx_msg.fields.type, midi_rx_msg.fields.data1,
				  midi_rx_msg.fields.data2);
		  if (midi_rx_msg.fields.type == CC){
			  if(midi_rx_msg.fields.data1 == MIDI_CC_TEST){
				  if (midi_rx_msg.fields.data2 > 63){
					  blink_interval_ms = BLINK_MIDI_CC;
				  }else{
					  blink_interval_ms = BLINK_MOUNTED;
				  }
			  } // if data1 = MIDI_CC_TEST
		  } // if type = CC
		  midi_rx_flag = 0;
	  }

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 20;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

int __io_putchar(int ch) {
    // Send character using ITM (SWO)
    // Port 0 is used by default.
    ITM_SendChar(ch);
    return ch;
}

int _write(int file, char *ptr, int len) {
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        __io_putchar(*ptr++);
    }
    return len;
}

// Invoked when device is mounted
void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

// Fills the buffer every ms with a sinusoidal signal.

void audio_task(void)
{
  static uint32_t start_ms = 0;
  uint32_t curr_ms = tusb_time_millis_api();

  if (start_ms == curr_ms) {
    return;
  }
  start_ms = curr_ms;
  static int32_t s = 0;

  // Fill the buffer with a sinusoid:
  for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
	//test_buffer_audio[i] = (int32_t)(AMPLITUDE * sinf(phase));
	s = (int32_t)((AMPLITUDE * sinf(phase))* 8388607.0f);

    usb_buffer[3*i + 0] = (uint8_t)(s & 0xFF);
    usb_buffer[3*i + 1] = (uint8_t)((s >> 8) & 0xFF);
    usb_buffer[3*i + 2] = (uint8_t)((s >> 16) & 0xFF);

    phase += phase_inc;

    if (phase > 2.0f * 3.1415926f) {
      phase -= 2.0f * 3.1415926f;
    }
  }
  // Writes AUDIO_BUFFER_SIZE samples:
  //tud_audio_write((uint32_t *) test_buffer_audio, sizeof(test_buffer_audio));
  tud_audio_write((uint8_t *) usb_buffer, 144);
}

/*
 * Function called when there is a MIDI packet to receive.
 *
 */
void tud_midi_rx_cb(uint8_t itf){
	midi_rx_flag = 1;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if (tusb_time_millis_api() - start_ms < blink_interval_ms) {
    return; // not enough time
  }
  start_ms += blink_interval_ms;

  HAL_GPIO_WritePin(LED_ONBOARD_GPIO_Port, LED_ONBOARD_Pin, led_state);
  //board_led_write(led_state);
  led_state = 1 - led_state;// toggle
}

#if 0 // Not used for now, but may be useful.
/* Timer interrupt callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM11){
		flag_tim11 = 1;
	} // if htim->Instance == TIM11
}
#endif
#if 0
/**
  * @brief Rx Transfer completed callbacks
  * @param huart: uart handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART2)
	{
		usart_received_done_flag = 1;
	}
    //HAL_UART_Transmit(&huart1,  (uint8_t *)aRxBuffer, RECEIVED_BYTES,0xFFFF);
}
#endif

void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
  system_ticks++;
}

uint32_t tusb_time_millis_api(void) {
  return system_ticks;
}

/*
 *  Configure MPU for the region used by USB, preventing it to
 *  be cached.
 * */
void MPU_Config_USB_D2_SRAM(void)
{
	  MPU_Region_InitTypeDef MPU_InitStruct = {0};

	  HAL_MPU_Disable();

	  // ============================
	  // REGION - USB / DMA SRAM
	  // ============================
	  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
	  MPU_InitStruct.BaseAddress = 0x30000000;
	  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;

	  MPU_InitStruct.SubRegionDisable = 0x00;
	  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
	  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

	  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

	  MPU_InitStruct.Enable = MPU_REGION_ENABLE;

	  HAL_MPU_ConfigRegion(&MPU_InitStruct);

	  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
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
#ifdef USE_FULL_ASSERT
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
