/**
 * @file modules.c
 * @brief Implementacja funkcji sterujących modułami pojazdu.
 * @date Jan 30, 2025
 * @author DeathLock
 */

#include "modules.h"
#include <string.h>
#include <stdlib.h>

/** @brief Odbiornik USART2 (HC-05): przerwanie tylko buforuje bajty.
 *  Komendy wykonuje watek ProcessHeartBeat, nigdy przerwanie UART.
 */
uint8_t rxData[1];
char rxBuff[64];
uint8_t rxIdx = 0;

#define RX_COMMAND_QUEUE_LENGTH 16U
static volatile uint8_t rxCommandQueue[RX_COMMAND_QUEUE_LENGTH][5];
static volatile uint8_t rxQueueHead = 0U;
static volatile uint8_t rxQueueTail = 0U;

/* Liczniki diagnostyczne do STM32CubeIDE -> Live Expressions. */
volatile uint32_t uart2_rx_bytes = 0U;
volatile uint32_t uart2_rx_commands = 0U;
volatile uint32_t uart2_rx_processed = 0U;
volatile uint32_t uart2_rx_errors = 0U;
volatile uint32_t uart2_rx_overruns = 0U;
volatile uint32_t uart2_rx_dropped = 0U;
volatile uint32_t uart2_rx_rearm_failures = 0U;
volatile uint32_t uart2_last_error = 0U;
/* Tick ostatniej przetworzonej poprawnej komendy/heartbeat (PC -> STM). */
volatile uint32_t uart2_last_valid_tick = 0U;
volatile uint32_t uart2_link_timeout_count = 0U;
static const uint8_t uart2_ready_frame[10] = {'R','C','R','E','A','D','Y','0','1','!'};

static void UART2_QueueCommand(void)
{
    uint8_t tail = rxQueueTail;
    uint8_t nextTail = (uint8_t)((tail + 1U) % RX_COMMAND_QUEUE_LENGTH);

    if (nextTail == rxQueueHead) {
        uart2_rx_dropped++;
        return;
    }

    for (uint8_t i = 0U; i < 4U; ++i)
        rxCommandQueue[tail][i] = (uint8_t)rxBuff[i];
    rxCommandQueue[tail][4] = 0U;
    __DMB(); /* Publikuj element kolejki dopiero po zapisaniu calej komendy. */
    rxQueueTail = nextTail;
    uart2_rx_commands++;
}

static void UART2_ConsumeByte(uint8_t byte)
{
    /* Wyszukujemy poczatek CMDx/INIT po dowolnej utracie bajtu. */
    switch (rxIdx) {
    case 0:
        if (byte == 'C' || byte == 'I' || byte == 'P') {
            rxBuff[0] = (char)byte;
            rxIdx = 1U;
        }
        return;
    case 1:
        if ((rxBuff[0] == 'C' && byte == 'M') ||
            (rxBuff[0] == 'I' && byte == 'N') ||
            (rxBuff[0] == 'P' && byte == 'I')) {
            rxBuff[1] = (char)byte;
            rxIdx = 2U;
            return;
        }
        break;
    case 2:
        if ((rxBuff[0] == 'C' && byte == 'D') ||
            (rxBuff[0] == 'I' && byte == 'I') ||
            (rxBuff[0] == 'P' && byte == 'N')) {
            rxBuff[2] = (char)byte;
            rxIdx = 3U;
            return;
        }
        break;
    case 3:
        if ((rxBuff[0] == 'I' && byte == 'T') ||
            (rxBuff[0] == 'P' && byte == 'G') ||
            (rxBuff[0] == 'C' &&
             ((byte >= '0' && byte <= '9') ||
              byte == 'A' || byte == 'B' || byte == 'C'))) {
            rxBuff[3] = (char)byte;
            UART2_QueueCommand();
            rxIdx = 0U;
            return;
        }
        break;
    default:
        break;
    }

    /* Ponowne szukanie synchronizacji po blednym znaku. */
    rxIdx = 0U;
    if (byte == 'C' || byte == 'I' || byte == 'P') {
        rxBuff[0] = (char)byte;
        rxIdx = 1U;
    }
}

void UART2_ProcessPendingCommands(void)
{
    uint8_t command[5];
    while (rxQueueHead != rxQueueTail) {
        uint8_t head = rxQueueHead;
        __DMB();
        for (uint8_t i = 0U; i < 5U; ++i)
            command[i] = rxCommandQueue[head][i];
        __DMB();
        rxQueueHead = (uint8_t)((head + 1U) % RX_COMMAND_QUEUE_LENGTH);
        uint32_t errorsBefore = uart2_rx_errors;
        uart2_last_valid_tick = HAL_GetTick();
        ProcessCommand(command);
        /* Jesli IRQ przerwal wykonywanie komendy bledem RX, zatrzymaj ponownie. */
        if (uart2_rx_errors != errorsBefore && engineOn) {
            ResetMotor(&motorA);
            ResetMotor(&motorB);
            motorA.actual_PWM = 0;
            motorB.actual_PWM = 0;
        }
        uart2_rx_processed++;
    }
}

void UART2_EnsureReception(void)
{
    /* Gdy uruchomienie odbioru zawiedzie, ponow probe z watku RTOS. */
    if (huart2.RxState == HAL_UART_STATE_READY) {
        if (HAL_UART_Receive_IT(&huart2, rxData, sizeof(rxData)) != HAL_OK)
            uart2_rx_rearm_failures++;
    }
}

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
    /* Nowa sesja: nie kontynuuj ruchu z poprzedniej sesji HC-05. */
    if (engineOn) {
        Command_Engine(); /* Zatrzymuje PWM, enkodery oraz TIM6. */
    }
    ResetMotor(&motorA);
    ResetMotor(&motorB);
    motorA.actual_PWM = 0;
    motorB.actual_PWM = 0;
    uart2_last_valid_tick = HAL_GetTick();
    HAL_UART_Transmit(&huart2, (uint8_t*)uart2_ready_frame, 10U, 50U);
}

/** Odmierzamy zdalną dostępność STM32 także przy wyłączonym napędzie. */
void Command_Ping(void)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)uart2_ready_frame, 10U, 50U);
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
        motorA.actual_PWM = 0;
        motorB.actual_PWM = 0;

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

	HAL_UART_Transmit(&huart3, (uint8_t*)"\nENGINE COMMAND\r\n", 17, 20U);
}

/** @brief Przełącza światła. */
void Command_Lights(void)
{
    HAL_GPIO_TogglePin(LIGHTS_GPIO_Port, LIGHTS_Pin);
	HAL_UART_Transmit(&huart3, (uint8_t*)"\nLIGHTS COMMAND\r\n", 17, 20U);
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

	HAL_UART_Transmit(&huart3, (uint8_t*)"\nHORN COMMAND\r\n", 15, 20U);
}

/** @brief Wymusza hamowanie silników (ABS). */
void Command_ABS(void)
{
	ResetMotor(&motorA);
	ResetMotor(&motorB);
    motorA.actual_PWM = 0;
    motorB.actual_PWM = 0;
	HAL_UART_Transmit(&huart3, (uint8_t*)"\nABS COMMAND\r\n", 14, 20U);
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
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR0 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, DEFAULT, 0);
		    motor_set_speed(&motorB, DEFAULT, 0);
            /* STOP: zeruj PWM bez czyszczenia licznikow enkoderow.
             * ResetMotor() kasuje takze TIM3/TIM8->CNT, czego oryginal
             * w tym miejscu NIE robil. */
            motorA.actual_PWM = 0;
            motorB.actual_PWM = 0;
            __HAL_TIM_SET_COMPARE(motorA.motorFront, motorA.axisTimer, 0);
            __HAL_TIM_SET_COMPARE(motorA.motorBack, motorA.axisTimer, 0);
            __HAL_TIM_SET_COMPARE(motorB.motorFront, motorB.axisTimer, 0);
            __HAL_TIM_SET_COMPARE(motorB.motorBack, motorB.axisTimer, 0);
		    break;

			case 2:
		    // Operacja dla MOTORF
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR1 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, FRONT, 93);
		    motor_set_speed(&motorB, FRONT, 100);
		    break;

			case 3:
		    // Operacja dla MOTORFR
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR2 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, FRONT, 50);
		    motor_set_speed(&motorB, FRONT, 100);
		    break;

			case 4:
		    // Operacja dla MOTORR
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR3 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, BACK, 100);
		    motor_set_speed(&motorB, FRONT, 100);
		    break;

			case 5:
		    // Operacja dla MOTORBR
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR4 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, BACK, 50);
		    motor_set_speed(&motorB, BACK, 100);
		    break;

			case 6:
		    // Operacja dla MOTORB
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR5 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, BACK, 100);
		    motor_set_speed(&motorB, BACK, 100);
		    break;

			case 7:
		    // Operacja dla MOTORBL
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR6 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, BACK, 100);
		    motor_set_speed(&motorB, BACK, 50);
		    break;

			case 8:
		    // Operacja dla MOTORL
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR7 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, FRONT, 100);
		    motor_set_speed(&motorB, BACK, 100);
		    break;

			case 9:
		    // Operacja dla MOTORFL
		    HAL_UART_Transmit(&huart3, (uint8_t *)"\nMOTOR8 selected\r\n", 19, 20U);
		    motor_set_speed(&motorA, FRONT, 100);
		    motor_set_speed(&motorB, FRONT, 50);
		    break;

			default:
		    break;

		}

	}

	else HAL_UART_Transmit(&huart3, (uint8_t *)"\nInvalid MOTOR mode\r\n", 22, 20U);
}

/**
 * @brief Przetwarza otrzymaną komendę.
 * @param _cmd Ciąg znaków zawierający komendę.
 */
void ProcessCommand(uint8_t* _cmd)
{

	if (strcmp((char*)_cmd, "INIT") == 0)
		Command_Init();

	else if (strcmp((char*)_cmd, "PING") == 0)
		Command_Ping();

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

	else HAL_UART_Transmit(&huart3, (uint8_t *)"\nUnknown command\r\n", 18, 20U);
}

/**
 * @brief Obsługa przerwania odbioru UART.
 * @param _huart Uchwyt struktury UART.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* _huart)
{
    if (_huart->Instance != USART2)
        return;

    /* Zapamietaj bajt ZANIM ponownie uzyjemy bufora rxData. */
    uint8_t byte = rxData[0];
    uart2_rx_bytes++;

    /* Odbior musi byc aktywny mozliwie szybko; bez blokujacego TX. */
    if (HAL_UART_Receive_IT(&huart2, rxData, sizeof(rxData)) != HAL_OK)
        uart2_rx_rearm_failures++;

    UART2_ConsumeByte(byte);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* _huart)
{
    if (_huart->Instance != USART2)
        return;

    uint32_t error = _huart->ErrorCode;
    uart2_last_error = error;
    uart2_rx_errors++;
    if ((error & HAL_UART_ERROR_ORE) != 0U)
        uart2_rx_overruns++;

    /* Nie czekaj na kolejna komende STOP po bledzie odbiornika. */
    rxIdx = 0U;
    rxQueueHead = rxQueueTail; /* Odrzuc niespozyte, potencjalnie stare komendy. */
    if (engineOn) {
        ResetMotor(&motorA);
        ResetMotor(&motorB);
        motorA.actual_PWM = 0;
        motorB.actual_PWM = 0;
    }

    /* HAL wyłącza RX dla ORE. Po bledzie nieblokujacym RX moze nadal dzialac. */
    if (_huart->RxState == HAL_UART_STATE_READY) {
        if (HAL_UART_Receive_IT(_huart, rxData, sizeof(rxData)) != HAL_OK)
            uart2_rx_rearm_failures++;
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
