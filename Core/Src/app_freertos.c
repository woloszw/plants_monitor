/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "printf.h"
#include "adc.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for sensAnalog */
osThreadId_t sensAnalogHandle;
const osThreadAttr_t sensAnalog_attributes = {
  .name = "sensAnalog",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for sensUART */
osThreadId_t sensUARTHandle;
const osThreadAttr_t sensUART_attributes = {
  .name = "sensUART",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 128 * 4
};
/* Definitions for sensI2C */
osThreadId_t sensI2CHandle;
const osThreadAttr_t sensI2C_attributes = {
  .name = "sensI2C",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
uint32_t readADCData(ADC_HandleTypeDef _adcHandle)
{
	uint32_t analogReadValue;
//	if(osSemaphoreAcquire(ADCSemaphoreHandle, 100)==osOK)
//	{
	HAL_ADC_Start(&_adcHandle);
	HAL_ADC_PollForConversion(&_adcHandle, 1000);

	analogReadValue = (uint32_t) HAL_ADC_GetValue(&_adcHandle);
	HAL_ADC_Stop(&_adcHandle);
//	}
//	if(osSemaphoreRelease(ADCSemaphoreHandle) == osOK)
		return analogReadValue;
//	else
//		return 0;
}
/* USER CODE END FunctionPrototypes */

void sensAnalogTask(void *argument);
void sensUARTTask(void *argument);
void sensI2CTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

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
  /* creation of sensAnalog */
  sensAnalogHandle = osThreadNew(sensAnalogTask, NULL, &sensAnalog_attributes);

  /* creation of sensUART */
  sensUARTHandle = osThreadNew(sensUARTTask, NULL, &sensUART_attributes);

  /* creation of sensI2C */
  sensI2CHandle = osThreadNew(sensI2CTask, NULL, &sensI2C_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_sensAnalogTask */
/**
  * @brief  Function implementing the sensAnalog thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_sensAnalogTask */
void sensAnalogTask(void *argument)
{
  /* USER CODE BEGIN sensAnalogTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END sensAnalogTask */
}

/* USER CODE BEGIN Header_sensUARTTask */
/**
* @brief Function implementing the sensUART thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_sensUARTTask */
void sensUARTTask(void *argument)
{
  /* USER CODE BEGIN sensUARTTask */
	uint32_t ddata;


  /* Infinite loop */
  for(;;)
  {
	 ddata = readADCData(hadc1);
	 printf("%d \n \r", ddata);


	  osDelay(1000);
  }
  /* USER CODE END sensUARTTask */
}

/* USER CODE BEGIN Header_sensI2CTask */
/**
* @brief Function implementing the sensI2C thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_sensI2CTask */
void sensI2CTask(void *argument)
{
  /* USER CODE BEGIN sensI2CTask */
  /* Infinite loop */
  for(;;)
  {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    osDelay(1000);
  }
  /* USER CODE END sensI2CTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void _putchar(char character)
{
  // send char to console etc.
	HAL_UART_Transmit(&hlpuart1, (uint8_t*) &character, 1, 1000);
}
/* USER CODE END Application */

