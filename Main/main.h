/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define CYCLES_PER_SIGNAL   500
#define LED_THRESHOLD       700
#define SENSITIVITY         40.0f
#define SERIAL_INTERVAL_MS  50

// Pins

// Oscillator for metal detection; GREEN WIRE
#define OSCILLATOR_PIN      GPIO_PIN_0
#define OSCILLATOR_PORT     GPIOB

// PWM Pin for noise generation, uses tim2; WHITE WIRE
#define NOISE_PIN 			GPIO_PIN_15
#define NOISE_PORT			GPIOA




#define LED_PIN             GPIO_PIN_7
#define LED_PORT            GPIOC

// Pins for Ultrasonic Sensor
#define ECHO_PIN 			GPIO_PIN_1
#define ECHO_PORT           GPIOA
#define TRIG_PIN 			GPIO_PIN_1
#define TRIG_PORT           GPIOB


#define NSLEEP_PIN			GPIO_PIN_9
#define NSLEEP_PORT			GPIOC

#define LIGHT_PIN			GPIO_PIN_12
#define LIGHT_PORT			GPIOA


#define BT_TXD_PIN			GPIO_PIN_5
#define BT_TXD_PORT			GPIOC

#define BT_RXD_PIN			GPIO_PIN_10
#define BT_RXD_PORT			GPIOB




#define ENABLE_METAL_DETECTION 		1
#define ENABLE_OBJ_DETECTION 		1
#define TOGGLE_LED 					0
#define ENABLE_MOTOR_SIGNAL 		0
#define ENABLE_BT_MOTOR 			1
#define ENABLE_BT_OBJ				1

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
