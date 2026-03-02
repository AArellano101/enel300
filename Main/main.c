/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Metal detector - STM32 port of Arduino code
  *                   Oscillator signal → PA0 (EXTI rising edge)
  *                   LED               → PA6
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile uint32_t cycleCount      = 0;
volatile uint32_t lastSignalTime  = 0;
volatile uint32_t signalTimeDelta = 0;
volatile uint8_t  firstSignal     = 1;
volatile uint32_t storedTimeDelta = 0;
volatile uint32_t interruptCount  = 0;

uint32_t lastPrintTime = 0;
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN 0 */

// DWT microsecond timer - equivalent to Arduino micros()
static void DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t micros(void)
{
  return DWT->CYCCNT / (SystemCoreClock / 1000000);
}

// Redirect printf to UART2
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  /* USER CODE BEGIN 2 */
  DWT_Init();
  /* USER CODE END 2 */

  MX_GPIO_Init();
  MX_USART2_UART_Init();

  printf("Calibrating - keep metal away from coil...\r\n");

  while (1)
  {
    /* USER CODE BEGIN WHILE */
    int32_t diff = (int32_t)((int32_t)(storedTimeDelta - signalTimeDelta) * SENSITIVITY);

    // Drive LED on PA6
    HAL_GPIO_WritePin(LED_PORT, LED_PIN,
      diff > LED_THRESHOLD ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Serial print every SERIAL_INTERVAL_MS
    uint32_t now = HAL_GetTick();
    if (now - lastPrintTime >= SERIAL_INTERVAL_MS)
    {
      lastPrintTime = now;
      if (storedTimeDelta == 0)
      {
        printf("Calibrating... Interrupts: %lu\r\n", interruptCount);
      }
      else
      {
        printf("Baseline: %lu us  |  Current: %lu us  |  Diff: %ld  |  Metal: %s\r\n",
          storedTimeDelta,
          signalTimeDelta,
          diff,
          diff > LED_THRESHOLD ? "YES" : "no");
      }
    }
    /* USER CODE END WHILE */
  }
}

/* USER CODE BEGIN 4 */
// Fires on every rising edge from oscillator on PA0
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == OSCILLATOR_PIN)
  {
    interruptCount++;
    cycleCount++;

    if (cycleCount >= CYCLES_PER_SIGNAL)
    {
      cycleCount = 0;

      uint32_t currentTime = micros();
      signalTimeDelta = currentTime - lastSignalTime;
      lastSignalTime  = currentTime;

      if (firstSignal)
      {
        firstSignal = 0;
      }
      else if (storedTimeDelta == 0)
      {
        // Store baseline - keep metal away from coil on startup
        storedTimeDelta = signalTimeDelta;
      }
    }
  }
}
/* USER CODE END 4 */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = 16;
  RCC_OscInitStruct.PLL.PLLN            = 336;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ            = 2;
  RCC_OscInitStruct.PLL.PLLR            = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance          = USART2;
  huart2.Init.BaudRate     = 115200;
  huart2.Init.WordLength   = UART_WORDLENGTH_8B;
  huart2.Init.StopBits     = UART_STOPBITS_1;
  huart2.Init.Parity       = UART_PARITY_NONE;
  huart2.Init.Mode         = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // LED on PA6 - output
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = LED_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

  // Oscillator input on PA0 - rising edge interrupt with pulldown
  GPIO_InitStruct.Pin  = OSCILLATOR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(OSCILLATOR_PORT, &GPIO_InitStruct);

  // Onboard LED LD2 on PA5
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = LD2_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  // B1 button on PC13
  GPIO_InitStruct.Pin  = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // Enable NVIC interrupts
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line){}
#endif
