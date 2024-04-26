/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VREF 3.3065 // напряжение �?ОН
#define NVAR 4095 // максимальное число вариаций 2^12-1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const int NUM_READ = 10;  // количество усреднений для средних арифм. фильтров(выборка точек)
float k = 0.1;  // коэффициент фильтрации, 0.0-1.0 чем меньше, тем плавнее результат
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch,FILE *f)
{
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,HAL_MAX_DELAY);
	return ch;
}
int fgetc(FILE *f)
{
	uint8_t ch;
	HAL_UART_Receive( &huart1,(uint8_t*)&ch,1, HAL_MAX_DELAY );
	return ch;
}
// понятное бегущее среднее арифметическое
float runMiddleArifm(float newVal)// принимает новое значение
{  
  static uint8_t idx = 0;             // индекс
  static float valArray[NUM_READ];    // массив(буфер)
  valArray[idx] = newVal;             // пишем каждый раз в новую ячейку
  if (++idx >= NUM_READ) 
      idx = 0;     // перезаписывая самое старое значение
  float average = 0;                  // обнуляем среднее
  for (int i = 0; i < NUM_READ; i++) 
    average += valArray[i];           // суммируем
  return (float)average / NUM_READ;   // возвращаем
}
//оптимальное бегущее среднее арифметическое 
float runMiddleArifmOptim(float newVal) 
{
  static int t = 0;
  static float vals[NUM_READ];
  static float average = 0;
  if (++t >= NUM_READ) 
      t = 0; // перемотка t
  average -= vals[t];// вычитаем старое
  average += newVal;// прибавляем новое
  vals[t] = newVal; // запоминаем в массив
  return ((float)average / NUM_READ);
}
// экспонцеанальное бегущее среднее
float expRunningAverage(float newVal) 
{
  static float filVal = 0;
  filVal += (newVal - filVal) * k;//фильтрованное += (новое - фильтрованное) * коэффициент
  return filVal;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//uint16_t rawValues[3];
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
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
    HAL_TIM_Base_Start(&htim3);//запуск таймера
	HAL_ADC_Start_IT(&hadc1);//запуск АЦП в режиме прерываний
	//HAL_ADCEx_InjectedStart_IT(&hadc1);//запуск АЦП в режиме прерываний для инжектированных каналов
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){};
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//GPIOA->BSRR|=GPIO_BSRR_BS7;//установить в логическую единицу port A pin 7
		//GPIOA->BSRR|=GPIO_BSRR_BR7;//установить в логический ноль port A pin 7
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) // функция вызывается автоматически по окончанию преобразования
{
	uint32_t CODE=HAL_ADC_GetValue(&hadc1);//результат в кодах
	float VALUE=VREF/NVAR*CODE;//результат в вольтах
    /*float SUM = 0;// локальная переменная sum
    for (int i = 0; i < NUM_READ; i++)// согласно количеству усреднений
    SUM += VALUE;// суммируем значения с любого датчика в переменную sum
	printf("%f\r\n",SUM / NUM_READ);*/
    //printf("%f\r\n",VALUE);
    float EXP_AVERAGE=expRunningAverage(VALUE);
    printf("%f\r\n",EXP_AVERAGE);
	HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_7);//инвертируем состояние порта
	HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);//инвертируем состояние порта, на который выведен Led
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

#ifdef  USE_FULL_ASSERT
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
