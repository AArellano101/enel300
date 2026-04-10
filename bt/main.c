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
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint16_t joy1 = 0;
uint16_t joy2 = 0;
uint16_t rawValues[2];
int16_t motorL = 0;
int16_t motorR = 0;
uint8_t headlights = 0;
char msg[32];

uint8_t rx_byte;
char buffer[32];
uint8_t buffer_idx = 0;
char rx_line[32];
volatile uint8_t line_ready = 0;
char distanceBuf[32];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE BEGIN 0 */

// ─── LCD low-level helpers ───────────────────────────────────────────────────

static void LCD_Pulse_Enable(void)
{
    HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void LCD_Send_Nibble(uint8_t nibble)
{
    HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_Pulse_Enable();
}

static void LCD_Send_Byte(uint8_t byte, uint8_t isData)
{
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, isData ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_Send_Nibble(byte >> 4);
    LCD_Send_Nibble(byte & 0x0F);
}

void LCD_Init(void)
{
    HAL_Delay(50);

    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    LCD_Send_Nibble(0x03); HAL_Delay(5);
    LCD_Send_Nibble(0x03); HAL_Delay(1);
    LCD_Send_Nibble(0x03); HAL_Delay(1);

    LCD_Send_Nibble(0x02); HAL_Delay(1);

    LCD_Send_Byte(0x28, 0); HAL_Delay(1);
    LCD_Send_Byte(0x0C, 0); HAL_Delay(1);
    LCD_Send_Byte(0x06, 0); HAL_Delay(1);
    LCD_Send_Byte(0x01, 0); HAL_Delay(2);
}

void LCD_Clear(void)
{
    LCD_Send_Byte(0x01, 0);
    HAL_Delay(2);
}

void LCD_Set_Cursor(uint8_t row, uint8_t col)
{
    // Row 0 starts at 0x80, row 1 starts at 0xC0
    uint8_t addr = (row == 0 ? 0x80 : 0xC0) + col;
    LCD_Send_Byte(addr, 0);
    HAL_Delay(1);
}

// ─── Main display function ───────────────────────────────────────────────────

/**
 * @brief  Display a string on the LCD at the current cursor position.
 * @param  text: null-terminated string to display
 */
void LCD_Display(const char *text)
{
    while (*text)
    {
        LCD_Send_Byte((uint8_t)*text++, 1);
        HAL_Delay(1);
    }
}

void Check_Distance_Display(void)
{
  static GPIO_PinState lastState = GPIO_PIN_SET;
  GPIO_PinState currentState = HAL_GPIO_ReadPin(GPIOC, r_distance_Pin);

  if (line_ready)
  {
    strncpy(distanceBuf, rx_line, sizeof(distanceBuf));
    line_ready = 0;
  }

  if (lastState == GPIO_PIN_SET && currentState == GPIO_PIN_RESET)
  {
    int len = snprintf(msg, sizeof(msg), "DIST:%s\n", distanceBuf);
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, len, HAL_MAX_DELAY);
  }
  lastState = currentState;
}

void Compute_MotorValues(int16_t *motorL, int16_t *motorR)
{
  joy1 = rawValues[1];
  joy2 = rawValues[0];

  if (joy1 >= 2000 && joy1 <= 2200)
    *motorL = 0;
  else if (joy1 > 2200)
    *motorL = (int16_t)(((int32_t)joy1 - 2200) * 100 / (4000 - 2200));
  else
    *motorL = (int16_t)(((int32_t)joy1 - 2000) * 100 / (2000 - 30));

  if (joy2 >= 2000 && joy2 <= 2200)
    *motorR = 0;
  else if (joy2 > 2200)
    *motorR = (int16_t)(((int32_t)joy2 - 2200) * 100 / (4000 - 2200));
  else
    *motorR = (int16_t)(((int32_t)joy2 - 2000) * 100 / (2000 - 30));

  if (*motorL > 100)  *motorL =  100;
  if (*motorL < -100) *motorL = -100;
  if (*motorR > 100)  *motorR =  100;
  if (*motorR < -100) *motorR = -100;

  *motorL *= -1;
  *motorR *= -1;

}

void Print_MotorValues(int16_t motorL, int16_t motorR)
{
  int len = snprintf(msg, sizeof(msg), "L:%d R:%d HL:%d\r\n",
                     motorL, motorR, headlights);
  HAL_UART_Transmit(&huart2, (uint8_t *)msg, len, HAL_MAX_DELAY);
}

void Send_MotorValues(int16_t motorL, int16_t motorR)
{
  static GPIO_PinState lastState = GPIO_PIN_SET;

  GPIO_PinState currentState = HAL_GPIO_ReadPin(turn_headlights_GPIO_Port, turn_headlights_Pin);
  if (lastState == GPIO_PIN_SET && currentState == GPIO_PIN_RESET)
    headlights ^= 1;
  lastState = currentState;

  char buf[32];
  int len = snprintf(buf, sizeof(buf), "L:%d,R:%d,HL:%d\n",
                     motorL, motorR, headlights);
  HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);


}
// Redirect printf to UART2
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)rawValues, 2);
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  HAL_UART_Transmit(&huart2, (uint8_t *)"BOOT\n", 5, HAL_MAX_DELAY);

  LCD_Init();
  LCD_Set_Cursor(0, 0);
//  LCD_Display("Hello World");     // row 0
//  HAL_Delay(2000);

  LCD_Set_Cursor(1, 0);
  LCD_Display("Distance: --");    // row 1

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_7, GPIO_PIN_SET);
//	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
//	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
//	  HAL_Delay(86400000);
	  	  		if (line_ready)
	  		{
	  		    char local[24];
	  		    int l;

	  		    __disable_irq();
	  		    strncpy(local, rx_line, sizeof(local));
	  		    local[sizeof(local) - 1] = '\0';
	  		    line_ready = 0;
	  		    __enable_irq();

	  		    //printf("RX: %s\r\n", local);

	  		    if (sscanf(local, "%d", &l) == 1)
	  		    {
	  		    	//printf("%d\r\n",l);
	  		    	int whole = l / 100;
	  		    	int frac  = l % 100;

	  		    	// UART (PuTTY)
	  		    	printf("dist %d.%02d cm\r\n", whole, frac);

	  		    	// LCD
	  		    	char lcdBuf[16];
	  		    	snprintf(lcdBuf, sizeof(lcdBuf), "Dist:%d.%02d", whole, frac);
	  		    	LCD_Clear();
	  		    	LCD_Set_Cursor(1, 0);
	  		    	LCD_Display("Dist:        ");
	  		    	LCD_Set_Cursor(1, 5);
	  		    	LCD_Display(lcdBuf + 5);


	  		    	//HAL_UART_Transmit(&huart2, (uint8_t *)msg, len, HAL_MAX_DELAY);
	  		    }
	  		    else
	  		    {
	  		        printf("Bad packet: %s\r\n", local);
	  		    }
	  		}

	  Compute_MotorValues(&motorL, & motorR);
	  Print_MotorValues(motorL, motorR);
	  Send_MotorValues(motorL, motorR);    // transmit on UART1 via HC-05


	  HAL_Delay(100);

//	  Print_JoystickValues();
//	  Send_Value(-124);
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RS_Pin|D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, E_Pin|D4_Pin|D6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RS_Pin D7_Pin */
  GPIO_InitStruct.Pin = RS_Pin|D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : r_distance_Pin */
  GPIO_InitStruct.Pin = r_distance_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(r_distance_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : D5_Pin */
  GPIO_InitStruct.Pin = D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(D5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : E_Pin D4_Pin D6_Pin */
  GPIO_InitStruct.Pin = E_Pin|D4_Pin|D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : turn_headlights_Pin */
  GPIO_InitStruct.Pin = turn_headlights_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(turn_headlights_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)

    {
        char c = (char)rx_byte;


        if (c == '\r')
        {
            // ignore carriage return
        }
        else if (c == '\n')
        {

            buffer[buffer_idx] = '\0';

            strncpy(rx_line, buffer, sizeof(rx_line));
            rx_line[sizeof(rx_line) - 1] = '\0';

            buffer_idx = 0;
            line_ready = 1;
        }
        else
        {
            if (buffer_idx < sizeof(buffer) - 1)
            {
                buffer[buffer_idx++] = c;
            }
            else
            {
                // overflow protection: reset buffer
                buffer_idx = 0;
            }
        }

        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

/* USER CODE END 4 */

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
