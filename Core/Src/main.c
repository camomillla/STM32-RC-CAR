/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "atc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
ATC_HandleTypeDef ESP;
int resp = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if (huart->Instance == USART2) {
		ATC_IdleLineCallback(&ESP, Size);
	}
}

typedef struct
{
	int previous_error; 		//Poprzedni błąd dla członu różniczkującego
	int total_error;		//Suma uchybów dla członu całkującego
	float Kp;			//Wzmocnienie członu proporcjonalnego
	float Ki;			//Wzmocnienie członu całkującego*/
	float Kd;			//Wzmocnienie członu różniczkującego*/
	int anti_windup_limit;		//Anti-Windup - ograniczenie członu całkującego*/
} PID;

void pid_init(PID *pid_data, float kp_init, float ki_init, float kd_init, int anti_windup_limit_init)
{
	pid_data->previous_error = 0;
	pid_data->total_error = 0;

	pid_data->Kp = kp_init;
	pid_data->Ki = ki_init;
	pid_data->Kd = kd_init;

	pid_data->anti_windup_limit = anti_windup_limit_init;
}

void pid_reset(PID *pid_data)
{
	pid_data->total_error = 0;
	pid_data->previous_error = 0;
}

int pid_calculate(PID *pid_data, int setpoint, int process_variable)
{
	int error;
	float p_term, i_term, d_term;

	error = setpoint - process_variable;		//obliczenie uchybu
	pid_data->total_error += error;			//sumowanie uchybu

	p_term = (float)(pid_data->Kp * error);		//odpowiedź członu proporcjonalnego
	i_term = (float)(pid_data->Ki * pid_data->total_error);	//odpowiedź członu całkującego
	d_term = (float)(pid_data->Kd * (error - pid_data->previous_error));//odpowiedź członu różniczkującego

	if(i_term >= pid_data->anti_windup_limit) i_term = pid_data->anti_windup_limit;	//Anti-Windup - ograniczenie odpowiedzi członu całkującego
	else if(i_term <= -pid_data->anti_windup_limit) i_term = -pid_data->anti_windup_limit;

	pid_data->previous_error = error;	//aktualizacja zmiennej z poprzednią wartością błędu

	return (int)(p_term + i_term + d_term);		//odpowiedź regulatora
}

#define MOTOR_A_Kp					3
#define MOTOR_A_Ki					0.05
#define MOTOR_A_Kd					0.05
#define MOTOR_A_ANTI_WINDUP			1

#define ENCODER_RESOLUTION			3
#define TIMER_CONF_BOTH_EDGE_T1T2	4
#define MOTOR_GEAR					50

#define	TIMER_FREQENCY				20
#define	SECOND_IN_MINUTE			60

typedef struct
{
	TIM_HandleTypeDef *timer;	//timer obsługujący enkoder silnika

	uint16_t resolution;		//rozdzielczość silnika

	int pulse_count;		//zliczone impulsy
	int measured_speed;		//obliczona prędkość silnika
	int set_speed;			//zadana prędkość silnika

	int actual_PWM;			//wartość PWM

	PID pid_controller;
} MOTOR;

MOTOR motorA;

void motor_init(MOTOR *m, TIM_HandleTypeDef *tim)
{
	m->timer = tim;
	m->resolution = ENCODER_RESOLUTION * TIMER_CONF_BOTH_EDGE_T1T2 * MOTOR_GEAR;

	m->pulse_count = 0;
	m->measured_speed = 0;
	m->set_speed = 0;
    m->actual_PWM = 0;
}

void motor_calculate_speed(MOTOR *m)
{
	motor_update_count(m);

	m->measured_speed = (m->pulse_count * TIMER_FREQENCY * SECOND_IN_MINUTE) / m->resolution;

	int output = pid_calculate(&(m->pid_controller), m->set_speed, m->measured_speed);

	m->actual_PWM += output;

	if(m->actual_PWM >= 0)
	{
		//drv8835_set_motorA_direction(CW);
		drv8835_set_motorA_speed(m->actual_PWM);
		//__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, m->actual_PWM);
		//__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 500);

	}
	else
	{
		//drv8835_set_motorA_direction(CCW);
		//drv8835_set_motorA_speed(-m->actual_PWM);
	}
}

void motor_update_count(MOTOR *m)
{
	m->pulse_count = (int16_t)__HAL_TIM_GET_COUNTER(m->timer);
	__HAL_TIM_SET_COUNTER(m->timer, 0);
}

void motor_set_speed(MOTOR *m, int set_speed)
{
	if(set_speed != m->set_speed)
		pid_reset(&(m->pid_controller));

	m->set_speed = set_speed;
}

void drv8835_set_motorA_speed(uint16_t speed)
{
	if(speed >= htim2.Instance->ARR)
		speed = htim2.Instance->ARR;

	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, speed);
}

void drv8835_init()
{
	//drv8835_mode_control(Phase_Enable_Mode);
	//drv8835_set_motorA_direction(CCW);
	drv8835_set_motorA_speed(0);

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6)
	{
		motor_calculate_speed(&motorA);
	}
}


/*=================================*/

void Set_PWM_Frequency(uint32_t frequency) {
    if (frequency == 0) {
        HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2); // Zatrzymanie PWM
        return;
    }

    uint32_t timer_clock = 96000000; // 96 MHz zegar bazowy
    uint32_t prescaler = htim12.Init.Prescaler + 1;
    uint32_t period = (timer_clock / (prescaler * frequency)) - 1;

    __HAL_TIM_SET_AUTORELOAD(&htim2, period);
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, period / 2); // Wypełnienie 50%
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2); // Start PWM
}

// Funkcja callback obsługująca przychodzące dane
void ATC_ReceiveCallback(const char *data) {
    // Wyślij dane na UART3, aby były widoczne w terminalu
    HAL_UART_Transmit(&huart3, (uint8_t *)data, strlen(data), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);  // Dodaj nową linię
}

int hornOn = 0;

void ProcessCommand(uint8_t* cmd) {
	// Sprawdź, czy komenda to LIGHTS
	    if (strcmp((char*)cmd, "LIGHTS") == 0) {
	        HAL_GPIO_TogglePin(LIGHTS_GPIO_Port, LIGHTS_Pin);
	    }

	    else if (strcmp((char*)cmd, "HORN") == 0) {
	    	if (!hornOn) {
		    	Set_PWM_Frequency(1000);
		    	hornOn = 1;
	    	}
	    	else {
	    		Set_PWM_Frequency(0);
	    		hornOn = 0;
	    	}
	    }

	    // Obsługa komend MOTORX
	    else if (strncmp((char*)cmd, "MOTOR", 5) == 0) { // Sprawdź, czy zaczyna się od "MOTOR"
	        char* modeStr = (char*)cmd + 5; // Wskaźnik na część po "MOTOR"
	        int mode = atoi(modeStr);       // Zamiana cyfry trybu na liczbę całkowitą

	        if (mode >= 0 && mode <= 8) {   // Sprawdzenie, czy tryb mieści się w zakresie 0-8
	            switch (mode) {
	                case 0:
	                    // Operacja dla MOTOR0
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR0 selected\r\n", 18, HAL_MAX_DELAY);
	                    motor_set_speed(&motorA, 0);
	                    break;
	                case 1:
	                    // Operacja dla MOTORF
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR1 selected\r\n", 18, HAL_MAX_DELAY);
	                    motor_set_speed(&motorA, 100);
	                    break;
	                case 2:
	                    // Operacja dla MOTORFR
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR2 selected\r\n", 18, HAL_MAX_DELAY);
	                    motor_set_speed(&motorA, 75);
	                    break;
	                case 3:
	                    // Operacja dla MOTORR
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR3 selected\r\n", 18, HAL_MAX_DELAY);
	                    break;
	                case 4:
	                    // Operacja dla MOTORBR
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR4 selected\r\n", 18, HAL_MAX_DELAY);
	                    break;
	                case 5:
	                    // Operacja dla MOTORB
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR5 selected\r\n", 18, HAL_MAX_DELAY);
	                    break;
	                case 6:
	                    // Operacja dla MOTORBL
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR6 selected\r\n", 18, HAL_MAX_DELAY);
	                    break;
	                case 7:
	                    // Operacja dla MOTORL
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR7 selected\r\n", 18, HAL_MAX_DELAY);
	                    break;
	                case 8:
	                    // Operacja dla MOTORFL
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"MOTOR8 selected\r\n", 18, HAL_MAX_DELAY);
	                    break;
	                default:
	                    break; // Nie powinno wystąpić
	            }
	        } else {
	            HAL_UART_Transmit(&huart3, (uint8_t *)"Invalid MOTOR mode\r\n", 21, HAL_MAX_DELAY);
	        }
	    }
	    // Nieznana komenda
	    else {
	        HAL_UART_Transmit(&huart3, (uint8_t *)"Unknown command\r\n", 17, HAL_MAX_DELAY);
	    }
}

// Funkcja obsługująca przetwarzanie danych przychodzących przez ESP
void ProcessIncomingData() {
	char *response = NULL; // Wskaźnik na odebrane dane
	    while (1) {
	        // Oczekiwanie na odpowiedź zawierającą +IPD
	        int result = ATC_Receive(&ESP, &response, 5000, 1, "+IPD,");
	        if (result > 0 && response != NULL) {
	            // Przetwarzanie odpowiedzi zawierającej +IPD
	            char *ipdStart = strstr(response, "+IPD,");
	            if (ipdStart != NULL) {
	                // Znajdź początek danych (po dwukropku ':')
	                char *dataStart = strchr(ipdStart, ':');
	                if (dataStart != NULL) {
	                    dataStart++; // Przesuń za dwukropek, aby przejść do danych
	                    // Wyślij dane na UART3
	                    HAL_UART_Transmit(&huart3, (uint8_t *)dataStart, strlen(dataStart), HAL_MAX_DELAY);
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY); // Nowa linia dla przejrzystości
	                    ProcessCommand((uint8_t *)dataStart);
	                }
	            }
	            ATC_RxFlush(&ESP); // Wyczyść bufor dla nowych danych
	        }

	        // Wywołanie głównej pętli ATC
	        ATC_Loop(&ESP);
	    }
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
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_TIM5_Init();
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */

  ATC_Init(&ESP, &huart2, 512, "ESP");
  ATC_SendReceive(&ESP, "AT\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&ESP, "AT+CWMODE=1\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&ESP, "AT+CIPMUX=1\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&ESP, "AT+CIPMODE=1\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&ESP, "AT+CIPSERVER=1,80\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&ESP, "AT+CWJAP=\"DeathLock\",\"\"\r\n", 10000, NULL, 10000, 0);

  const char *readyMsg = "STM32 ready to receive data from ESP...\r\n";
  HAL_UART_Transmit(&huart3, (uint8_t *)readyMsg, strlen(readyMsg), HAL_MAX_DELAY);

  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_ALL);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  HAL_TIM_Base_Start_IT(&htim6);

  HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);

  drv8835_init();
  motor_init(&motorA, &htim4);
  pid_init(&(motorA.pid_controller), MOTOR_A_Kp, MOTOR_A_Ki, MOTOR_A_Kd, MOTOR_A_ANTI_WINDUP);
  ProcessIncomingData();


  //Set_PWM_Frequency(1000); // BUZZER

  /* USER CODE END 2 */

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
