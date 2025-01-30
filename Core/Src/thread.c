/*
 * thread.c
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#include "thread.h"

/** @brief Atrybuty wątków RTOS */
const osThreadAttr_t heartBeatHandle_attributes = {
  .name = "heartBeat",
  .stack_size = 128 * 4,
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
osThreadId_t testScenario;
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
void DistanceIndicator(void*) {

	while (1) {

		if (!indicationOverride)
			continue;

		if (hornOn)
			continue;

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
		}
	}
}

/**
 * @brief Wątek obsługujący czujnik odległości.
 * @details Odczytuje czas trwania impulsu echa i przelicza go na odległość.
 * @param[in] argument Nie używane.
 */
void DistanceSensor(void*) {
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

        Distance = (Value2 - Value1) * 0.034 / 2;
    }
}

/**
 * @brief Wątek wysyłający dane telemetryczne.
 * @details Wysyła prędkości silników i odległość przez UART co 100ms.
 * @param[in] argument Nie używane.
 */
void ProcessHeartBeat(void* argument) {
    for (;;) {

    	if (!engineOn)
    		continue;

    	int16_t data[5] = {motorA.measured_speed, motorB.measured_speed, motorA.set_speed, motorB.set_speed, Distance};
    	HAL_UART_Transmit(&huart2, (uint8_t*)data, sizeof(data), HAL_MAX_DELAY);

        osDelay(100);
    }
}
