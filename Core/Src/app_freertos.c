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
#include "bmp280.h"
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
#define MOISTURE_SENSOR_TASK_ID 1
#define MOISTURE_MIN 500
#define MOISTURE_MAX 2000

#define BMP280_TASK_ID 2
#define TEMPERATURE_MIN 15
#define TEMPERATURE_MAX 50

#define INSOLATION_TASK_ID 3

#define RAIN_DETECTION_TASK_ID 4


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern BMP280_t Bmp280;

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
/* Definitions for infoTask */
osThreadId_t infoTaskHandle;
const osThreadAttr_t infoTask_attributes = {
  .name = "infoTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for sensGPIO */
osThreadId_t sensGPIOHandle;
const osThreadAttr_t sensGPIO_attributes = {
  .name = "sensGPIO",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for queueToUART */
osMessageQueueId_t queueToUARTHandle;
const osMessageQueueAttr_t queueToUART_attributes = {
  .name = "queueToUART"
};
/* Definitions for queueToInfo */
osMessageQueueId_t queueToInfoHandle;
const osMessageQueueAttr_t queueToInfo_attributes = {
  .name = "queueToInfo"
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
	if(HAL_ADC_PollForConversion(&_adcHandle, 1000)==HAL_OK)
		analogReadValue = (uint32_t) HAL_ADC_GetValue(&_adcHandle);


	HAL_ADC_Stop(&_adcHandle);

	return analogReadValue;
}

void SetChannel(uint32_t Channel)
{
	  ADC_ChannelConfTypeDef sConfig = {0};

	  /** Configure Regular Channel
	    */
	    sConfig.Channel = Channel;
	    sConfig.Rank = ADC_REGULAR_RANK_1;
	    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	    sConfig.SingleDiff = ADC_SINGLE_ENDED;
	    sConfig.OffsetNumber = ADC_OFFSET_NONE;
	    sConfig.Offset = 0;
	    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	    {
	      Error_Handler();
	    }
}

/* USER CODE END FunctionPrototypes */

void sensAnalog01Task(void *argument);
void sensUARTTask(void *argument);
void sensI2CTask(void *argument);
void sensAnalog02Task(void *argument);
void StartInfoTask(void *argument);
void sensGPIOTask(void *argument);

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

  /* creation of queueToInfo */
  queueToInfoHandle = osMessageQueueNew (16, sizeof(SensorData), &queueToInfo_attributes);

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

  /* creation of infoTask */
  infoTaskHandle = osThreadNew(StartInfoTask, NULL, &infoTask_attributes);

  /* creation of sensGPIO */
  sensGPIOHandle = osThreadNew(sensGPIOTask, NULL, &sensGPIO_attributes);

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
	// Dirt Moisture sensor
	SensorData analog1task =	{	.id = MOISTURE_SENSOR_TASK_ID, .data = 123};

  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(ADC_SemaphoreHandle, 100);
	  SetChannel(ADC_CHANNEL_1);
	  analog1task.data = readADCData(hadc1);
	  osSemaphoreRelease(ADC_SemaphoreHandle);

	  osMessageQueuePut(queueToUARTHandle, (SensorData*)&analog1task, 0, osWaitForever);
	  osMessageQueuePut(queueToInfoHandle, (SensorData*)&analog1task, 0, osWaitForever);
	  osDelay(10000);
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
		printf("%d:%d \n \r", receivedData.id, receivedData.data);
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
	//Bmp280 - pressure and temperature sensor
	extern BMP280_t Bmp280;
	float Temp, Pressure;
	SensorData i2ctask = {.id = BMP280_TASK_ID, .data = 0};

  /* Infinite loop */
  for(;;)
  {
	BMP280_ReadPressureAndTemperature(&Bmp280, &Pressure, &Temp);
	i2ctask.data = Pressure;
	osMessageQueuePut(queueToUARTHandle, (SensorData*)&i2ctask, 0, osWaitForever);
	i2ctask.data = Temp;
	osMessageQueuePut(queueToUARTHandle, (SensorData*)&i2ctask, 0, osWaitForever);
	osMessageQueuePut(queueToInfoHandle, (SensorData*)&i2ctask, 0, osWaitForever);

	osDelay(10000);
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
	// Insolation sensor
	SensorData analog2task =	{.id = INSOLATION_TASK_ID, .data = 0	};

  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(ADC_SemaphoreHandle, 100);
	  SetChannel(ADC_CHANNEL_2);
	  analog2task.data = readADCData(hadc1);
	  osSemaphoreRelease(ADC_SemaphoreHandle);

	  osMessageQueuePut(queueToUARTHandle, (SensorData*)&analog2task, 0, osWaitForever);
	  osDelay(10000);
  }
  /* USER CODE END sensAnalog02Task */
}

/* USER CODE BEGIN Header_StartInfoTask */
/**
* @brief Function implementing the infoTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartInfoTask */
void StartInfoTask(void *argument)
{
  /* USER CODE BEGIN StartInfoTask */
	SensorData infoData;
  /* Infinite loop */
  for(;;)
  {
	if(osOK == osMessageQueueGet(queueToInfoHandle, (SensorData*)&infoData, 0, osWaitForever))
	  	{
	  		//printf("\n%d:%d \n \r", infoData.id, receivedData.data);
			switch(infoData.id)
			{
			case BMP280_TASK_ID:
							//Because SensorData type has only one data field, measures temperature and pressure are contantly
							//overwriting .data. This is the way to determine whether the current data is temperature (usually below 50 Celcius degrees) or atmosperic pressure.
							if (infoData.data >= 100){
								break;}


							if(infoData.data > TEMPERATURE_MAX)
								printf("Too hot\n \r");
							else if (infoData.data < TEMPERATURE_MIN) {
								printf("Too cold\n \r");
							}
							else {
								printf("Optimal temperature\n \r");
							}
							break;


			case MOISTURE_SENSOR_TASK_ID:
				if(infoData.data > MOISTURE_MAX)
					printf("Dirt too wet\n \r");
				else if (infoData.data < MOISTURE_MIN) {
					printf("Dirt too dry\n \r");
				}
				else {
					printf("Dirt is moist enough\n \r");
				}
				break;

			case RAIN_DETECTION_TASK_ID:
				if(infoData.data == GPIO_PIN_SET)
					printf("It is raining\n \r");

			default:
				//I have noticed that with semaphore on uart at least 2 tasks must use printf.
				printf("");
				break;
			}


	  	}
    osDelay(2000);
  }
  /* USER CODE END StartInfoTask */
}

/* USER CODE BEGIN Header_sensGPIOTask */
/**
* @brief Function implementing the sensGPIO thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_sensGPIOTask */
void sensGPIOTask(void *argument)
{
  /* USER CODE BEGIN sensGPIOTask */
	SensorData gpiotask =
		{
				.id = RAIN_DETECTION_TASK_ID,
				.data = 0
		};
  /* Infinite loop */
  for(;;)
  {
	 gpiotask.data = (uint32_t)HAL_GPIO_ReadPin(GPIO_IN_GPIO_Port, GPIO_IN_Pin);

	 osMessageQueuePut(queueToUARTHandle, (SensorData*)&gpiotask, 0, osWaitForever);
	 osDelay(5000);
  }
  /* USER CODE END sensGPIOTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void _putchar(char character)
{
  // send char to console etc.
	osSemaphoreAcquire(UART_SemaphoreHandle, 100);
	HAL_UART_Transmit(&hlpuart1, (uint8_t*) &character, 1, 1000); //for debug only
	//HAL_UART_Transmit(&huart5, (uint8_t*) &character, 1, 1000);
	osSemaphoreRelease(UART_SemaphoreHandle);
}
/* USER CODE END Application */

