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
#include "stm32f1xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_STATUS_Pin GPIO_PIN_13
#define LED_STATUS_GPIO_Port GPIOC
#define ADC_LX_Pin GPIO_PIN_0
#define ADC_LX_GPIO_Port GPIOA
#define ADC_LY_Pin GPIO_PIN_1
#define ADC_LY_GPIO_Port GPIOA
#define ADC_RX_Pin GPIO_PIN_2
#define ADC_RX_GPIO_Port GPIOA
#define ADC_RY_Pin GPIO_PIN_3
#define ADC_RY_GPIO_Port GPIOA
#define ADC_LT_Pin GPIO_PIN_4
#define ADC_LT_GPIO_Port GPIOA
#define ADC_RT_Pin GPIO_PIN_5
#define ADC_RT_GPIO_Port GPIOA
#define KEY_R_STICK_Pin GPIO_PIN_6
#define KEY_R_STICK_GPIO_Port GPIOA
#define KEY_L_STICK_Pin GPIO_PIN_7
#define KEY_L_STICK_GPIO_Port GPIOA
#define KEY_RIGHT_Pin GPIO_PIN_0
#define KEY_RIGHT_GPIO_Port GPIOB
#define KEY_DOWN_Pin GPIO_PIN_1
#define KEY_DOWN_GPIO_Port GPIOB
#define KEY_LEFT_Pin GPIO_PIN_10
#define KEY_LEFT_GPIO_Port GPIOB
#define KEY_UP_Pin GPIO_PIN_11
#define KEY_UP_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_12
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_13
#define OLED_SDA_GPIO_Port GPIOB
#define KEY_RB_Pin GPIO_PIN_14
#define KEY_RB_GPIO_Port GPIOB
#define KEY_LB_Pin GPIO_PIN_15
#define KEY_LB_GPIO_Port GPIOB
#define RGB_DATA_Pin GPIO_PIN_8
#define RGB_DATA_GPIO_Port GPIOA
#define KEY_HOME_Pin GPIO_PIN_3
#define KEY_HOME_GPIO_Port GPIOB
#define KEY_MENU_Pin GPIO_PIN_4
#define KEY_MENU_GPIO_Port GPIOB
#define KEY_VIEW_Pin GPIO_PIN_5
#define KEY_VIEW_GPIO_Port GPIOB
#define KEY_Y_Pin GPIO_PIN_6
#define KEY_Y_GPIO_Port GPIOB
#define KEY_B_Pin GPIO_PIN_7
#define KEY_B_GPIO_Port GPIOB
#define KEY_A_Pin GPIO_PIN_8
#define KEY_A_GPIO_Port GPIOB
#define KEY_X_Pin GPIO_PIN_9
#define KEY_X_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
