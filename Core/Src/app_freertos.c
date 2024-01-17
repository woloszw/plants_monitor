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
typedef struct
{
uint8_t id; 	//Value used to identify which task has sent data
uint32_t data;  //Data e.g. from sensor
} SensorData;
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
/* Definitions for sensAnalog01 */
osThreadId_t sensAnalog01Handle;
const osThreadAttr_t sensAnalog01_attributes = {
  .name = "sensAnalog01",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for sensUART */
osThreadId_t sensUARTHandle;
const osThreadAttr_t sensUART_attributes = {
  .name = "sensUART",
  .priority = (osPriority_t) osPriorityAboveNormal7,
  .stack_size = 128 * 4
};
/* Definitions for sensI2C */
osThreadId_t sensI2CHandle;
const osThreadAttr_t sensI2C_attributes = {
  .name = "sensI2C",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for sensAnalog02 */
osThreadId_t sensAnalog02Handle;
const osThreadAttr_t sensAnalog02_attributes = {
  .name = "sensAnalog02",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for pumpTask */
osThreadId_t pumpTaskHandle;
const osThreadAttr_t pumpTask_attributes = {
  .name = "pumpTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for queueToUART */
osMessageQueueId_t queueToUARTHandle;
const osMessageQueueAttr_t queueToUART_attributes = {
  .name = "queueToUART"
};
/* Definitions for queueToPump */
osMessageQueueId_t queueToPumpHandle;
const osMessageQueueAttr_t queueToPump_attributes = {
  .name = "queueToPump"
};
/* Definitions for UART_Semaphore */
osSemaphoreId_t UART_SemaphoreHandle;
const osSemaphoreAttr_t UART_Semaphore_attributes = {
  .name = "UART_Semaphore"
};
/* Definitions for ADC_Semaphore */
osSemaphoreId_t ADC_SemaphoreHandle;
const osSemaphoreAttr_t ADC_Semaphore_attributes = {
  .name = "ADC_Semaphore"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
uint32_t readADCData(ADC_HandleTypeDef _adcHandle)
{
	uint32_t analogReadValue;

	HAL_ADC_Start(&_adcHandle);
	HAL_ADC_PollForConversion(&_adcHandle, 1000);

	analogReadValue = (uint32_t) HAL_ADC_GetValue(&_adcHandle);
	HAL_ADC_Stop(&_adcHandle);

	return analogReadValue;
}
/* USER CODE END FunctionPrototypes */

void sensAnalog01Task(void *argument);
void sensUARTTask(void *argument);
void sensI2CTask(void *argument);
void sensAnalog02Task(void *argument);
void StartPumpTask(void *argument);

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

  /* Create the semaphores(s) */
  /* creation of UART_Semaphore */
  UART_SemaphoreHandle = osSemaphoreNew(1, 0, &UART_Semaphore_attributes);

  /* creation of ADC_Semaphore */
  ADC_SemaphoreHandle = osSemaphoreNew(1, 0, &ADC_Semaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queueToUART */
  queueToUARTHandle = osMessageQueueNew (16, sizeof(SensorData), &queueToUART_attributes);

  /* creation of queueToPump */
  queueToPumpHandle = osMessageQueueNew (16, sizeof(uint16_t), &queueToPump_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of sensAnalog01 */
  sensAnalog01Handle = osThreadNew(sensAnalog01Task, NULL, &sensAnalog01_attributes);

  /* creation of sensUART */
  sensUARTHandle = osThreadNew(sensUARTTask, NULL, &sensUART_attributes);

  /* creation of sensI2C */
  sensI2CHandle = osThreadNew(sensI2CTask, NULL, &sensI2C_attributes);

  /* creation of sensAnalog02 */
  sensAnalog02Handle = osThreadNew(sensAnalog02Task, NULL, &sensAnalog02_attributes);

  /* creation of pumpTask */
  pumpTaskHandle = osThreadNew(StartPumpTask, NULL, &pumpTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_sensAnalog01Task */
/**
  * @brief  Function implementing the sensAnalog01 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_sensAnalog01Task */
void sensAnalog01Task(void *argument)
{
  /* USER CODE BEGIN sensAnalog01Task */
	SensorData analog1task =
	{
		.id = 1,
		.data = 123,
	};

  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(ADC_SemaphoreHandle, 100);
	  analog1task.data = readADCData(hadc1);
	  osSemaphoreRelease(ADC_SemaphoreHandle);
	  //analog1task.data = 4321;

	  if(osOK==osMessageQueuePut(queueToUARTHandle, (SensorData*)&analog1task, 0, osWaitForever))
	  {
		  //printf("Sending %d from %d \n \r", analog1task.data, analog1task.id);
	  }


    osDelay(1000);
  }
  /* USER CODE END sensAnalog01Task */
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
	SensorData receivedData;
  /* Infinite loop */
  for(;;)
  {
	if(osOK == osMessageQueueGet(queueToUARTHandle, (SensorData*)&receivedData, 0, osWaitForever))
	{
		printf("%d : %d \n \r", receivedData.id, receivedData.data);
	}
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
	osDelay(1);
  }
  /* USER CODE END sensI2CTask */
}

/* USER CODE BEGIN Header_sensAnalog02Task */
/**
* @brief Function implementing the sensAnalog02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_sensAnalog02Task */
void sensAnalog02Task(void *argument)
{
  /* USER CODE BEGIN sensAnalog02Task */
	SensorData analog2task =
	{
			.id = 2,
			.data = 0
	};

  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(ADC_SemaphoreHandle, 100);
	  analog2task.data = readADCData(hadc1);
	  osSemaphoreRelease(ADC_SemaphoreHandle);
//	  analog2task.data = 1234;
	  if(osOK==osMessageQueuePut(queueToUARTHandle, (SensorData*)&analog2task, 0, osWaitForever))
	  {
	  //printf("Sending %d from %d \n \r", analog2task.data, analog2task.id);

	  }
	  osDelay(1000);
  }
  /* USER CODE END sensAnalog02Task */
}

/* USER CODE BEGIN Header_StartPumpTask */
/**
* @brief Function implementing the pumpTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPumpTask */
void StartPumpTask(void *argument)
{
  /* USER CODE BEGIN StartPumpTask */
  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

//	  if(HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin))
//		  printf("LED ON \n \r");
//	  else
//		  printf("LED OFF \n \r");

	  osDelay(1000);
  }
  /* USER CODE END StartPumpTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void _putchar(char character)
{
  // send char to console etc.
	osSemaphoreAcquire(UART_SemaphoreHandle, 100);
	HAL_UART_Transmit(&hlpuart1, (uint8_t*) &character, 1, 1000);
	//HAL_UART_Transmit(&huart5, (uint8_t*) &character, 1, 1000);
	osSemaphoreRelease(UART_SemaphoreHandle);
}
/* USER CODE END Application */

