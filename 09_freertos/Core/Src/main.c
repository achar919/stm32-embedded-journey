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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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

ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* Definitions for KnobTask */
osThreadId_t KnobTaskHandle;
const osThreadAttr_t KnobTask_attributes = {
  .name = "KnobTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LoggerTask */
osThreadId_t LoggerTaskHandle;
const osThreadAttr_t LoggerTask_attributes = {
  .name = "LoggerTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */
volatile uint32_t knobValue = 0;   // written by KnobTask, read by the others
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
void StartKnobTask(void *argument);
void StartDisplayTask(void *argument);
void StartLoggerTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

uint32_t readKnobAverage(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < 8; i++)
    {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        sum += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
    }
    return sum / 8;
}

/* ---------- Mini OLED driver: 1.3" SH1106, 128x64, I2C (address 0x3C) ---------- */
/* Font: 6x8 glyphs adapted from stm32-ssd1306 by Aleksander Alekseev (MIT licence) */
#define OLED_ADDR     (0x3C << 1)   // 7-bit address shifted for HAL
#define OLED_XOFFSET  2             // SH1106 picture starts 2 columns in (use 0 for SSD1306)

static uint8_t oledBuf[128 * 8];    // frame buffer: 128 columns x 8 pages (8 rows each)

static const uint8_t font6x8[95][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // space
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00},  // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00},  // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00},  // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00},  // $
    {0x23, 0x13, 0x08, 0x64, 0x62, 0x00},  // %
    {0x36, 0x49, 0x56, 0x20, 0x50, 0x00},  // &
    {0x00, 0x08, 0x07, 0x03, 0x00, 0x00},  // '
    {0x00, 0x1C, 0x22, 0x41, 0x00, 0x00},  // (
    {0x00, 0x41, 0x22, 0x1C, 0x00, 0x00},  // )
    {0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x00},  // *
    {0x08, 0x08, 0x3E, 0x08, 0x08, 0x00},  // +
    {0x00, 0x00, 0x70, 0x30, 0x00, 0x00},  // ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x00},  // -
    {0x00, 0x00, 0x60, 0x60, 0x00, 0x00},  // .
    {0x20, 0x10, 0x08, 0x04, 0x02, 0x00},  // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00},  // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00},  // 1
    {0x72, 0x49, 0x49, 0x49, 0x46, 0x00},  // 2
    {0x21, 0x41, 0x49, 0x4D, 0x33, 0x00},  // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00},  // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00},  // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00},  // 6
    {0x41, 0x21, 0x11, 0x09, 0x07, 0x00},  // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00},  // 8
    {0x46, 0x49, 0x49, 0x29, 0x1E, 0x00},  // 9
    {0x00, 0x00, 0x14, 0x00, 0x00, 0x00},  // :
    {0x00, 0x40, 0x34, 0x00, 0x00, 0x00},  // ;
    {0x00, 0x08, 0x14, 0x22, 0x41, 0x00},  // <
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x00},  // =
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00},  // >
    {0x02, 0x01, 0x59, 0x09, 0x06, 0x00},  // ?
    {0x3E, 0x41, 0x5D, 0x59, 0x4E, 0x00},  // @
    {0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00},  // A
    {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00},  // B
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00},  // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00},  // D
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00},  // E
    {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00},  // F
    {0x3E, 0x41, 0x41, 0x51, 0x73, 0x00},  // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00},  // H
    {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00},  // I
    {0x20, 0x40, 0x41, 0x3F, 0x01, 0x00},  // J
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00},  // K
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00},  // L
    {0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x00},  // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00},  // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00},  // O
    {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00},  // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00},  // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00},  // R
    {0x26, 0x49, 0x49, 0x49, 0x32, 0x00},  // S
    {0x03, 0x01, 0x7F, 0x01, 0x03, 0x00},  // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00},  // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00},  // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00},  // W
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00},  // X
    {0x03, 0x04, 0x78, 0x04, 0x03, 0x00},  // Y
    {0x61, 0x59, 0x49, 0x4D, 0x43, 0x00},  // Z
    {0x00, 0x7F, 0x41, 0x41, 0x41, 0x00},  // [
    {0x02, 0x04, 0x08, 0x10, 0x20, 0x00},  // backslash
    {0x00, 0x41, 0x41, 0x41, 0x7F, 0x00},  // ]
    {0x04, 0x02, 0x01, 0x02, 0x04, 0x00},  // ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x00},  // _
    {0x00, 0x03, 0x07, 0x08, 0x00, 0x00},  // `
    {0x20, 0x54, 0x54, 0x78, 0x40, 0x00},  // a
    {0x7F, 0x28, 0x44, 0x44, 0x38, 0x00},  // b
    {0x38, 0x44, 0x44, 0x44, 0x28, 0x00},  // c
    {0x38, 0x44, 0x44, 0x28, 0x7F, 0x00},  // d
    {0x38, 0x54, 0x54, 0x54, 0x18, 0x00},  // e
    {0x00, 0x08, 0x7E, 0x09, 0x02, 0x00},  // f
    {0x18, 0x24, 0x24, 0x1C, 0x78, 0x00},  // g
    {0x7F, 0x08, 0x04, 0x04, 0x78, 0x00},  // h
    {0x00, 0x44, 0x7D, 0x40, 0x00, 0x00},  // i
    {0x20, 0x40, 0x40, 0x3D, 0x00, 0x00},  // j
    {0x7F, 0x10, 0x28, 0x44, 0x00, 0x00},  // k
    {0x00, 0x41, 0x7F, 0x40, 0x00, 0x00},  // l
    {0x7C, 0x04, 0x78, 0x04, 0x78, 0x00},  // m
    {0x7C, 0x08, 0x04, 0x04, 0x78, 0x00},  // n
    {0x38, 0x44, 0x44, 0x44, 0x38, 0x00},  // o
    {0x7C, 0x18, 0x24, 0x24, 0x18, 0x00},  // p
    {0x18, 0x24, 0x24, 0x18, 0x7C, 0x00},  // q
    {0x7C, 0x08, 0x04, 0x04, 0x08, 0x00},  // r
    {0x48, 0x54, 0x54, 0x54, 0x24, 0x00},  // s
    {0x04, 0x04, 0x3F, 0x44, 0x24, 0x00},  // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00},  // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00},  // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00},  // w
    {0x44, 0x28, 0x10, 0x28, 0x44, 0x00},  // x
    {0x4C, 0x10, 0x10, 0x10, 0x7C, 0x00},  // y
    {0x44, 0x64, 0x54, 0x4C, 0x44, 0x00},  // z
    {0x00, 0x08, 0x36, 0x41, 0x00, 0x00},  // {
    {0x00, 0x00, 0x77, 0x00, 0x00, 0x00},  // |
    {0x00, 0x41, 0x36, 0x08, 0x00, 0x00},  // }
    {0x02, 0x01, 0x02, 0x04, 0x02, 0x00},  // ~
};

static void oled_cmd(uint8_t c)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDR, 0x00, 1, &c, 1, HAL_MAX_DELAY);
}

void oled_init(void)
{
    const uint8_t init[] = {0xAE, 0xD5, 0xF0, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14,
                            0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xFF, 0xD9, 0x22, 0xDB, 0x20,
                            0xA4, 0xA6, 0xAF};
    HAL_Delay(100);
    for (uint8_t i = 0; i < sizeof(init); i++) oled_cmd(init[i]);
}

void oled_clear(void)
{
    memset(oledBuf, 0, sizeof(oledBuf));
}

void oled_pixel(uint8_t x, uint8_t y)
{
    if (x < 128 && y < 64) oledBuf[x + (y / 8) * 128] |= (1 << (y % 8));
}

void oled_char(uint8_t x, uint8_t y, char ch, uint8_t scale)
{
    if (ch < 32 || ch > 126) ch = '?';
    for (uint8_t col = 0; col < 6; col++)
        for (uint8_t row = 0; row < 8; row++)
            if (font6x8[ch - 32][col] & (1 << row))
                for (uint8_t dx = 0; dx < scale; dx++)
                    for (uint8_t dy = 0; dy < scale; dy++)
                        oled_pixel(x + col * scale + dx, y + row * scale + dy);
}

void oled_string(uint8_t x, uint8_t y, const char *s, uint8_t scale)
{
    while (*s) { oled_char(x, y, *s++, scale); x += 6 * scale; }
}

void oled_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    for (uint8_t x = x1; x <= x2; x++) { oled_pixel(x, y1); oled_pixel(x, y2); }
    for (uint8_t y = y1; y <= y2; y++) { oled_pixel(x1, y); oled_pixel(x2, y); }
}

void oled_fill_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    for (uint8_t x = x1; x <= x2; x++)
        for (uint8_t y = y1; y <= y2; y++) oled_pixel(x, y);
}

void oled_update(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        oled_cmd(0xB0 + page);                        // select page (8-pixel row band)
        oled_cmd(0x00 + (OLED_XOFFSET & 0x0F));       // column address, low half
        oled_cmd(0x10 + (OLED_XOFFSET >> 4));         // column address, high half
        HAL_I2C_Mem_Write(&hi2c1, OLED_ADDR, 0x40, 1, &oledBuf[page * 128], 128, HAL_MAX_DELAY);
    }
}
/* ---------- end of OLED driver ---------- */
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
  MX_TIM6_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE BEGIN 2 */
  /* USER CODE BEGIN 2 */
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("Lesson 09: FreeRTOS starting\r\n");

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  oled_init();
  oled_clear();
  oled_string(10, 28, "Starting RTOS", 1);
  oled_update();
  /* USER CODE END 2 */
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
  /* creation of KnobTask */
  KnobTaskHandle = osThreadNew(StartKnobTask, NULL, &KnobTask_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);

  /* creation of LoggerTask */
  LoggerTaskHandle = osThreadNew(StartLoggerTask, NULL, &LoggerTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8399;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 4999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartKnobTask */
/**
  * @brief  Function implementing the KnobTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartKnobTask */
void StartKnobTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
	  knobValue = readKnobAverage();
	     __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, knobValue * 999 / 4095);
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
	  char line[20];
	  for(;;)
	  {
	    uint32_t v = knobValue;

	    oled_clear();
	    oled_string(0, 0, "FreeRTOS: 3 tasks", 1);   // small text (scale 1)

	    sprintf(line, "%4lu", v);
	    oled_string(0, 20, line, 2);                 // big number (scale 2)

	    oled_rect(0, 52, 127, 63);                   // bar outline
	    oled_fill_rect(0, 52, v * 127 / 4095, 63);   // bar fill
	    oled_update();                               // send to the screen

	    osDelay(200);
	  }
  /* USER CODE END StartDisplayTask */
}

/* USER CODE BEGIN Header_StartLoggerTask */
/**
* @brief Function implementing the LoggerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLoggerTask */
void StartLoggerTask(void *argument)
{
  /* USER CODE BEGIN StartLoggerTask */
  for(;;)
  {
    printf("[%lu s] knob = %lu\r\n", HAL_GetTick() / 1000, knobValue);
    osDelay(1000);
  }
  /* USER CODE END StartLoggerTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
