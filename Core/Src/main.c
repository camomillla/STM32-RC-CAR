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
#include "cmsis_os.h"
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
#include "FreeRTOS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
ATC_HandleTypeDef HC05;
int resp = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if (huart->Instance == USART2) {
		ATC_IdleLineCallback(&HC05, Size);
	}
}

typedef struct
{
	int previous_error;
	int total_error;
	float Kp;
	float Ki;
	float Kd;
	int anti_windup_limit;
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

	error = setpoint - process_variable;
	pid_data->total_error += error;

	p_term = (float)(pid_data->Kp * error);
	i_term = (float)(pid_data->Ki * pid_data->total_error);
	d_term = (float)(pid_data->Kd * (error - pid_data->previous_error));

	if(i_term >= pid_data->anti_windup_limit) i_term = pid_data->anti_windup_limit;
	else if(i_term <= -pid_data->anti_windup_limit) i_term = -pid_data->anti_windup_limit;

	pid_data->previous_error = error;

	return (int)(p_term + i_term + d_term);
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
	TIM_HandleTypeDef *timer;

	uint16_t resolution;

	int pulse_count;
	int measured_speed;
	int set_speed;

	int actual_PWM;

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
		drv8835_set_motorA_speed(m->actual_PWM);

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
        HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
        return;
    }

    uint32_t timer_clock = 96000000;
    uint32_t prescaler = htim12.Init.Prescaler + 1;
    uint32_t period = (timer_clock / (prescaler * frequency)) - 1;

    __HAL_TIM_SET_AUTORELOAD(&htim2, period);
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, period / 2);
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
}

void ATC_ReceiveCallback(const char *data) {

    HAL_UART_Transmit(&huart3, (uint8_t *)data, strlen(data), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
}

int hornOn = 0;

void ProcessCommand(uint8_t* cmd) {

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

	    else if (strncmp((char*)cmd, "MOTOR", 5) == 0) {
	        char* modeStr = (char*)cmd + 5;
	        int mode = atoi(modeStr);

	        if (mode >= 0 && mode <= 8) {
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
	                    break;
	            }
	        } else {
	            HAL_UART_Transmit(&huart3, (uint8_t *)"Invalid MOTOR mode\r\n", 21, HAL_MAX_DELAY);
	        }
	    }

	    else {
	        HAL_UART_Transmit(&huart3, (uint8_t *)"Unknown command\r\n", 17, HAL_MAX_DELAY);
	    }
}

void ProcessIncomingData(void* argument) {
	char *response = NULL;
	    while (1) {

	        int result = ATC_Receive(&HC05, &response, 5000, 1, "+IPD,");
	        if (result > 0 && response != NULL) {

	            char *ipdStart = strstr(response, "+IPD,");
	            if (ipdStart != NULL) {

	                char *dataStart = strchr(ipdStart, ':');
	                if (dataStart != NULL) {
	                    dataStart++;

	                    HAL_UART_Transmit(&huart3, (uint8_t *)dataStart, strlen(dataStart), HAL_MAX_DELAY);
	                    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
	                    ProcessCommand((uint8_t *)dataStart);
	                }
	            }
	            ATC_RxFlush(&HC05);
	        }

	        ATC_Loop(&HC05);
	        osDelay(50);
	    }
}

void ProcessHeartBeat(void* argument) {
    char heartbeatMessage[32];
    const uint8_t channel = 0;
    const int timeout = 5000;
    int lastSpeed = 0;

    while (1) {

    	if (lastSpeed == motorA.measured_speed)
    		continue;

    	lastSpeed = motorA.measured_speed;

        sprintf(heartbeatMessage, "HB:%d\r\n", motorA.measured_speed);
        char command[32];
        sprintf(command, "AT+CIPSEND=%d,%d\r\n", channel, strlen(heartbeatMessage));

        if (ATC_Send(&HC05, command, timeout)) {
            osDelay(50);

            if (!ATC_Send(&HC05, heartbeatMessage, timeout)) {
                printf("Błąd wysyłania heartbeat\n");
            }
        } else {
            printf("Błąd komendy AT+CIPSEND\n");
        }

        ATC_Loop(&HC05);

        osDelay(500);
    }
}

void TESTSCENARIO(void*) {

	while(1){
		if (HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin))
			break;
	}

	while(1) {
		motor_set_speed(&motorA, rand() % 100);
		osDelay(5000);
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

  ATC_Init(&HC05, &huart2, 2048, "HC05");
  ATC_SendReceive(&HC05, "AT\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&HC05, "AT+NAMERC-Car\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&HC05, "AT+PIN1609\r\n", 1000, NULL, 1000, 0);
  ATC_SendReceive(&HC05, "AT+BAUD8\r\n", 1000, NULL, 1000, 0);

  const char *readyMsg = "STM32 ready to receive data from HC05...\r\n";
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

  //Set_PWM_Frequency(1000); // BUZZER

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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
