/**
 * @file modules.c
 * @brief Implementacja funkcji sterujących modułami pojazdu.
 * @date Jan 30, 2025
 * @author DeathLock
 */

#include "modules.h"

/** @brief Bufor odbiorczy UART */
uint8_t rxData[1];
char rxBuff[64];
uint8_t rxIdx = 0;

/** @brief Zmienne do pomiaru odległości */
uint32_t pMillis;
uint32_t Value1;
uint32_t Value2;
uint32_t Distance;

/** @brief Flagi sterujące */
int hornOn = 0;
int indicationOverride = 0;
int engineOn = 0;

/**
 * @brief Ustawia częstotliwość PWM dla sygnału dźwiękowego.
 * @param _frequency Żądana częstotliwość w Hz.
 */
void Set_PWM_Frequency(uint32_t _frequency)
{

    if (_frequency == 0) {
        HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
        return;
    }

    uint32_t timer_clock = 96000000;
    uint32_t prescaler = htim12.Init.Prescaler + 1;
    uint32_t period = (timer_clock / (prescaler * _frequency)) - 1;

    __HAL_TIM_SET_AUTORELOAD(&htim12, period);
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, period / 2);
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
}

/** @brief Komenda inicjalizująca po nawiązaniu połączenia. */
void Command_Init(void)
{
	// Brak komunikatu o nawiązaniu połączenia
}

/** @brief Włącza lub wyłącza silnik. */
void Command_Engine(void)
{

	if (!engineOn) {
		HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
		HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
		HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
		HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_ALL);

		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
		HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);

		HAL_TIM_Base_Start_IT(&htim6);
		engineOn = 1;
	}

	else {
		ResetMotor(&motorA);
		ResetMotor(&motorB);

		HAL_TIM_Encoder_Stop(&htim1, TIM_CHANNEL_ALL);
		HAL_TIM_Encoder_Stop(&htim3, TIM_CHANNEL_ALL);
		HAL_TIM_Encoder_Stop(&htim4, TIM_CHANNEL_ALL);
		HAL_TIM_Encoder_Stop(&htim8, TIM_CHANNEL_ALL);

		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
		HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_4);

		HAL_TIM_Base_Stop_IT(&htim6);
		engineOn = 0;
	}

	HAL_UART_Transmit(&huart3, (uint8_t*)"\nENGINE COMMAND\r\n", 17, HAL_MAX_DELAY);
}

/** @brief Przełącza światła. */
void Command_Lights(void)
{
    HAL_GPIO_TogglePin(LIGHTS_GPIO_Port, LIGHTS_Pin);
	HAL_UART_Transmit(&huart3, (uint8_t*)"\nLIGHTS COMMAND\r\n", 17, HAL_MAX_DELAY);
}

/** @brief Włącza lub wyłącza klakson. */
void Command_Horn(void)
{

	if (!hornOn) {
		Set_PWM_Frequency(1000);
		hornOn = 1;
	}

	else {
		Set_PWM_Frequency(0);
		hornOn = 0;
	}

	HAL_UART_Transmit(&huart3, (uint8_t*)"\nHORN COMMAND\r\n", 15, HAL_MAX_DELAY);
}

/** @brief Wymusza hamowanie silników (ABS). */
void Command_ABS(void)
{
	ResetMotor(&motorA);
	ResetMotor(&motorB);
	HAL_UART_Transmit(&huart3, (uint8_t*)"\nABS COMMAND\r\n", 14, HAL_MAX_DELAY);
}

/**
 * @brief Obsługuje komendy sterujące silnikami.
 * @param _cmd Komenda w postaci ciągu znaków.
 */
void Command_Motors(uint8_t* _cmd)
{
	char* modeStr = (char*)_cmd + 3;
	int mode = atoi(modeStr);

	if (mode >= 1 && mode <= 9) {
		switch (mode) {

			case 1:
		    // Operacja dla MOTOR0
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR0 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, DEFAULT, 0);
		    motor_set_speed(&motorB, DEFAULT, 0);
		    break;

			case 2:
		    // Operacja dla MOTORF
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR1 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, FRONT, 93);
		    motor_set_speed(&motorB, FRONT, 100);
		    break;

			case 3:
		    // Operacja dla MOTORFR
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR2 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, FRONT, 50);
		    motor_set_speed(&motorB, FRONT, 100);
		    break;

			case 4:
		    // Operacja dla MOTORR
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR3 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, BACK, 100);
		    motor_set_speed(&motorB, FRONT, 100);
		    break;

			case 5:
		    // Operacja dla MOTORBR
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR4 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, BACK, 50);
		    motor_set_speed(&motorB, BACK, 100);
		    break;

			case 6:
		    // Operacja dla MOTORB
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR5 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, BACK, 100);
		    motor_set_speed(&motorB, BACK, 100);
		    break;

			case 7:
		    // Operacja dla MOTORBL
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR6 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, BACK, 100);
		    motor_set_speed(&motorB, BACK, 50);
		    break;

			case 8:
		    // Operacja dla MOTORL
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR7 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, FRONT, 100);
		    motor_set_speed(&motorB, BACK, 100);
		    break;

			case 9:
		    // Operacja dla MOTORFL
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR8 selected\r\n", 19, HAL_MAX_DELAY);
		    motor_set_speed(&motorA, FRONT, 100);
		    motor_set_speed(&motorB, FRONT, 50);
		    break;

			default:
		    break;

		}

	}

	else HAL_UART_Transmit(&huart3, (uint8_t *)"\nInvalid MOTOR mode\r\n", 22, HAL_MAX_DELAY);
}

/**
 * @brief Przetwarza otrzymaną komendę.
 * @param _cmd Ciąg znaków zawierający komendę.
 */
void ProcessCommand(uint8_t* _cmd)
{

	if (strcmp((char*)_cmd, "INIT") == 0)
		Command_Init();

	else if (strcmp((char*)_cmd, "CMD0") == 0)
		Command_Engine();

	else if (strcmp((char*)_cmd, "CMDA") == 0)
		Command_Lights();

	else if (strcmp((char*)_cmd, "CMDB") == 0)
		Command_Horn();

	else if (strcmp((char*)_cmd, "CMDC") == 0)
		Command_ABS();

	else if (strncmp((char*)_cmd, "CMD", 3) == 0)
		Command_Motors(_cmd);

	else HAL_UART_Transmit(&huart3, (uint8_t *)"\nUnknown command\r\n", 18, HAL_MAX_DELAY);
}

/**
 * @brief Obsługa przerwania odbioru UART.
 * @param _huart Uchwyt struktury UART.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* _huart)
{

	if (_huart->Instance==USART2) {
		HAL_UART_Transmit(&huart3, rxData, 1, HAL_MAX_DELAY);
		HAL_UART_Receive_IT(&huart2, &rxData, 1);
		rxBuff[rxIdx++] = rxData[0];

		if (rxIdx == 4) {
			ProcessCommand(&rxBuff);
			rxIdx = 0;
		}
	}
}

/**
 * @brief Obsługa przerwania przycisku GPIO.
 * @param GPIO_Pin Numer pinu, który wywołał przerwanie.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	static uint32_t lastPressTime = 0;

	if (GPIO_Pin == GPIO_PIN_13) {
		uint32_t currentTime = HAL_GetTick();

		if (currentTime - lastPressTime > 500) {  // 500 ms debounce
			indicationOverride = !indicationOverride;

			HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

			lastPressTime = currentTime;
		}
	}
}
