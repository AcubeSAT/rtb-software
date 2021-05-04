/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_dma.h"

#include "stm32h7xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdatomic.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern DAC_HandleTypeDef hdac1;
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim8;
extern SRAM_HandleTypeDef hsram1;
extern ADC_HandleTypeDef hadc1;

#define STATE_STRING_SIZE 10

#define STDOUT_BUFFER_SIZE 2048
#define UART_BUFFER_MAX 2048
extern uint8_t uart_buffer[UART_BUFFER_MAX];
extern atomic_uint uart_write;
extern atomic_uint uart_read;
extern bool uart_mutex;

extern enum State {
    none,
    canRX,
    canTX,
    canIdle
} state;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define UART_CONTROL "\x11"
#define UART_C_TIME "t"
#define UART_C_LATCHUP "l"
#define UART_C_MEASUREMENT "m"
#define UART_C_CANBITERROR "b"
#define UART_C_CANERROR "c"
#define UART_C_STATISTICS "s"
#define UART_C_MEMPROGRESS "p"
#define UART_C_MEMERROR "r"
#define UART_C_BOOT "o"

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void uart_command_received(const uint8_t* command, uint32_t len);
void state_to_string(enum State state, char * string);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SR_SER_Pin GPIO_PIN_2
#define SR_SER_GPIO_Port GPIOE
#define A1_Pin GPIO_PIN_0
#define A1_GPIO_Port GPIOC
#define SR_SRCLK_Pin GPIO_PIN_0
#define SR_SRCLK_GPIO_Port GPIOA
#define A0_Pin GPIO_PIN_3
#define A0_GPIO_Port GPIOA
#define LCL_TEST_Pin GPIO_PIN_7
#define LCL_TEST_GPIO_Port GPIOA
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define LCL_SET_Pin GPIO_PIN_8
#define LCL_SET_GPIO_Port GPIOC
#define LCL_RESET_Pin GPIO_PIN_9
#define LCL_RESET_GPIO_Port GPIOC
#define LCL_OUT_Pin GPIO_PIN_10
#define LCL_OUT_GPIO_Port GPIOC
#define LCL_OUT_EXTI_IRQn EXTI15_10_IRQn
#define RELAY_Pin GPIO_PIN_7
#define RELAY_GPIO_Port GPIOD
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define SR_RCLK_Pin GPIO_PIN_0
#define SR_RCLK_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
