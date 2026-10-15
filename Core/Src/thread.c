/*
 * thread.c
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#include "thread.h"
/* Prototypy odbiornika (w docelowym projekcie również w modules.h). */
void UART2_EnsureReception(void);
void UART2_ProcessPendingCommands(void);

/** @brief Atrybuty wątków RTOS */
const osThreadAttr_t heartBeatHandle_attributes = {
  .name = "heartBeat",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t distanceSensor_attributes = {
  .name = "distanceSensor",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t distanceIndicator_attributes = {
  .name = "distanceIndicator",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/** @brief Uchwyty wątków */
osThreadId_t heartBeatHandle;
osThreadId_t distanceSensorHandle;
osThreadId_t distanceIndicatorHandle;

/**
 * @brief Funkcja inicjalizująca i uruchamiająca wątki RTOS.
 */
void StartMultiThreads() {
	heartBeatHandle = osThreadNew(ProcessHeartBeat, NULL, &heartBeatHandle_attributes);
	distanceSensorHandle = osThreadNew(DistanceSensor, NULL, &distanceSensor_attributes);
	distanceIndicatorHandle = osThreadNew(DistanceIndicator, NULL, &distanceIndicator_attributes);
};

/**
 * @brief Wątek obsługujący wskaźnik odległości.
 * @details Steruje sygnalizacją świetlną oraz dźwiękową w zależności od zmierzonej odległości.
 * @param[in] argument Nie używane.
 */
void DistanceIndicator(void* argument) {

	while (1) {

        if (!indicationOverride || hornOn) {
            osDelay(10);
            continue;
        }

		if (Distance > 10 && Distance <= 30) {
		    Set_PWM_Frequency(1000);
		    HAL_GPIO_TogglePin(DISTANCE_INDICATOR_GPIO_Port, DISTANCE_INDICATOR_Pin);
		    osDelay(500);
		    Set_PWM_Frequency(0);
		    HAL_GPIO_TogglePin(DISTANCE_INDICATOR_GPIO_Port, DISTANCE_INDICATOR_Pin);
		    osDelay(500);
		}

		else if (Distance > 0 && Distance <= 10) {
		    Set_PWM_Frequency(1000);
		    HAL_GPIO_TogglePin(DISTANCE_INDICATOR_GPIO_Port, DISTANCE_INDICATOR_Pin);
		    osDelay(100);
		    Set_PWM_Frequency(0);
		    HAL_GPIO_TogglePin(DISTANCE_INDICATOR_GPIO_Port, DISTANCE_INDICATOR_Pin);
		    osDelay(100);
		}

		else {
		    HAL_GPIO_WritePin(DISTANCE_INDICATOR_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
            Set_PWM_Frequency(0);
            osDelay(20);
        }
    }
}

/**
 * @brief Wątek obsługujący czujnik odległości.
 * @details Odczytuje czas trwania impulsu echa i przelicza go na odległość.
 * @param[in] argument Nie używane.
 */
void DistanceSensor(void* argument) {
    /* Pojedynczy brak echa (0 cm) nie nadpisuje ostatniego poprawnego pomiaru.
       Przy dluzszym braku danych przyjmujemy bezpiecznie 1 cm, nie stara odleglosc. */
    const uint32_t distanceStaleTimeoutMs = 1000U;
    uint32_t lastValidDistanceTick = HAL_GetTick();
    Distance = 1U;  /* Brak jeszcze pomiaru: konserwatywna odleglosc. */

    HAL_TIM_Base_Start(&htim9);
    HAL_GPIO_WritePin(DETECTOR_TRIGGER_GPIO_Port, DETECTOR_TRIGGER_Pin, GPIO_PIN_RESET);

    while (1) {

        HAL_GPIO_WritePin(DETECTOR_TRIGGER_GPIO_Port, DETECTOR_TRIGGER_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COUNTER(&htim9, 0);
        while (__HAL_TIM_GET_COUNTER(&htim9) < 10);
        HAL_GPIO_WritePin(DETECTOR_TRIGGER_GPIO_Port, DETECTOR_TRIGGER_Pin, GPIO_PIN_RESET);

        pMillis = HAL_GetTick();
        while (!(HAL_GPIO_ReadPin(DETECTOR_ECHO_GPIO_Port, DETECTOR_ECHO_Pin)) && pMillis + 10 > HAL_GetTick());
        Value1 = __HAL_TIM_GET_COUNTER(&htim9);

        pMillis = HAL_GetTick();
        while ((HAL_GPIO_ReadPin(DETECTOR_ECHO_GPIO_Port, DETECTOR_ECHO_Pin)) && pMillis + 50 > HAL_GetTick());
        Value2 = __HAL_TIM_GET_COUNTER(&htim9);

        uint32_t measuredDistance = (Value2 - Value1) * 0.034 / 2;
        uint32_t now = HAL_GetTick();
        if (measuredDistance > 0U) {
            Distance = measuredDistance;
            lastValidDistanceTick = now;
        }
        else if ((uint32_t)(now - lastValidDistanceTick) >= distanceStaleTimeoutMs) {
            Distance = 1U;  /* Czujnik niedostepny: nie zgłaszaj pustej drogi. */
        }
        /* Przy krotkim braku echa pozostaw poprzednia wartosc Distance. */
    }
}

/**
 * @brief Wątek wysyłający dane telemetryczne.
 * @details Wysyła prędkości silników i odległość przez UART co 100ms.
 * @param[in] argument Nie używane.
 */
volatile uint32_t diag_heartbeat_min_stack_words = 0xFFFFFFFFU;

void ProcessHeartBeat(void* argument) {
    uint32_t lastHeartbeat = HAL_GetTick();

    for (;;) {
        /* Przetwarzanie komend także przy wylaczonym napedzie (CMD0/INIT). */
        UART2_EnsureReception();
        UART2_ProcessPendingCommands();

        uint32_t now = HAL_GetTick();
        if (engineOn && (uint32_t)(now - lastHeartbeat) >= 100U) {
            int16_t data[5] = {motorA.measured_speed, motorB.measured_speed,
                               motorA.set_speed, motorB.set_speed, Distance};
            HAL_UART_Transmit(&huart2, (uint8_t*)data, sizeof(data), 50U);
            lastHeartbeat = now;
        }
        else if (!engineOn) {
            lastHeartbeat = now;
        }

        // Monitor minimum available thread stack space (FreeRTOS reports words).
        uint32_t freeWords = (uint32_t)uxTaskGetStackHighWaterMark(NULL);
        if (freeWords < diag_heartbeat_min_stack_words)
            diag_heartbeat_min_stack_words = freeWords;
        osDelay(2);
    }
}
