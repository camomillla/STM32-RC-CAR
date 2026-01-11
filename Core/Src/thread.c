/*
 * thread.c
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#include "thread.h"

static inline uint8_t time_reached(uint32_t now, uint32_t target)
{
    return (int32_t)(now - target) >= 0;
}

/* ===========================
 *  HeartBeat (telemetria)
 * =========================== */

static uint32_t hb_nextSendMs = 0;

/**
 * @brief Logika wysyłania danych telemetrycznych.
 * @details Wysyła prędkości silników i odległość przez UART co 100ms (bez RTOS).
 */
static void HeartBeat_Update(void)
{
    if (!engineOn) {
        hb_nextSendMs = HAL_GetTick();
        return;
    }

    uint32_t now = HAL_GetTick();
    if (!time_reached(now, hb_nextSendMs))
        return;

    int16_t data[5] = {
        (int16_t)motorA.measured_speed,
        (int16_t)motorB.measured_speed,
        (int16_t)motorA.set_speed,
        (int16_t)motorB.set_speed,
        (int16_t)Distance
    };

    HAL_UART_Transmit(&huart3, (uint8_t*)data, sizeof(data), HAL_MAX_DELAY);

    hb_nextSendMs = now + 100;
}

/* ===========================
 *  Czujnik odległości
 * =========================== */

typedef enum {
    DS_TRIG_PULSE = 0,
    DS_WAIT_ECHO_RISE,
    DS_WAIT_ECHO_FALL
} DistanceSensorState;

static DistanceSensorState ds_state = DS_TRIG_PULSE;

/**
 * @brief Logika pomiaru odległości czujnikiem ultradźwiękowym.
 * @details Implementacja w formie maszyny stanów (bez RTOS), aby nie blokować pętli głównej.
 *          Zachowuje te same timeouty co wersja RTOS: 10ms na zbocze narastające ECHO i 50ms na opadanie.
 */
static void DistanceSensor_Update(void)
{
    switch (ds_state) {

    case DS_TRIG_PULSE:
        HAL_GPIO_WritePin(DETECTOR_TRIGGER_GPIO_Port, DETECTOR_TRIGGER_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COUNTER(&htim9, 0);
        while (__HAL_TIM_GET_COUNTER(&htim9) < 10) { }
        HAL_GPIO_WritePin(DETECTOR_TRIGGER_GPIO_Port, DETECTOR_TRIGGER_Pin, GPIO_PIN_RESET);

        pMillis = HAL_GetTick();
        ds_state = DS_WAIT_ECHO_RISE;
        break;

    case DS_WAIT_ECHO_RISE: {
        uint32_t now = HAL_GetTick();
        if ((HAL_GPIO_ReadPin(DETECTOR_ECHO_GPIO_Port, DETECTOR_ECHO_Pin) == GPIO_PIN_SET) ||
            (now - pMillis >= 10)) {
            Value1 = __HAL_TIM_GET_COUNTER(&htim9);
            pMillis = now;
            ds_state = DS_WAIT_ECHO_FALL;
        }
        break;
    }

    case DS_WAIT_ECHO_FALL: {
        uint32_t now = HAL_GetTick();
        if ((HAL_GPIO_ReadPin(DETECTOR_ECHO_GPIO_Port, DETECTOR_ECHO_Pin) == GPIO_PIN_RESET) ||
            (now - pMillis >= 50)) {
            Value2 = __HAL_TIM_GET_COUNTER(&htim9);
            Distance = (uint32_t)(((float)(Value2 - Value1)) * 0.034f / 2.0f);
            ds_state = DS_TRIG_PULSE;
        }
        break;
    }

    default:
        ds_state = DS_TRIG_PULSE;
        break;
    }
}

/* ===========================
 *  Wskaźnik odległości
 * =========================== */

typedef enum {
    IND_PHASE_IDLE = 0,
    IND_PHASE_ON_WAIT,
    IND_PHASE_OFF_WAIT
} IndicatorPhase;

static IndicatorPhase ind_phase = IND_PHASE_IDLE;
static uint32_t ind_nextMs = 0;
static uint32_t ind_onDelayMs = 0;
static uint32_t ind_offDelayMs = 0;

/**
 * @brief Logika wskaźnika odległości.
 * @details Steruje sygnalizacją świetlną oraz dźwiękową w zależności od zmierzonej odległości (bez RTOS).
 *          Warunki i czasy (100/500ms) odpowiadają wersji RTOS.
 */
static void DistanceIndicator_Update(void)
{
    uint32_t now = HAL_GetTick();

    if (ind_phase != IND_PHASE_IDLE) {
        if (!time_reached(now, ind_nextMs))
            return;

        if (ind_phase == IND_PHASE_ON_WAIT) {
            Set_PWM_Frequency(0);
            HAL_GPIO_TogglePin(DISTANCE_INDICATOR_GPIO_Port, DISTANCE_INDICATOR_Pin);
            ind_nextMs = now + ind_offDelayMs;
            ind_phase = IND_PHASE_OFF_WAIT;
            return;
        }

        if (ind_phase == IND_PHASE_OFF_WAIT) {
            ind_phase = IND_PHASE_IDLE;
            return;
        }

        ind_phase = IND_PHASE_IDLE;
        return;
    }

    /* Początek nowego cyklu — odpowiada pojedynczemu przejściu pętli while(1) z wersji RTOS. */
    if (!indicationOverride)
        return;

    if (hornOn)
        return;

    if (Distance > 10 && Distance <= 30) {
        ind_onDelayMs = 500;
        ind_offDelayMs = 500;
    }
    else if (Distance > 0 && Distance <= 10) {
        ind_onDelayMs = 100;
        ind_offDelayMs = 100;
    }
    else {
        HAL_GPIO_WritePin(DISTANCE_INDICATOR_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
        Set_PWM_Frequency(0);
        return;
    }

    Set_PWM_Frequency(1000);
    HAL_GPIO_TogglePin(DISTANCE_INDICATOR_GPIO_Port, DISTANCE_INDICATOR_Pin);
    ind_nextMs = now + ind_onDelayMs;
    ind_phase = IND_PHASE_ON_WAIT;
}

/* ===========================
 *  API (bez RTOS)
 * =========================== */

/**
 * @brief Funkcja inicjalizująca i uruchamiająca logikę dawnych wątków RTOS.
 */
void StartMultiThreads(void)
{
    HAL_TIM_Base_Start(&htim9);
    HAL_GPIO_WritePin(DETECTOR_TRIGGER_GPIO_Port, DETECTOR_TRIGGER_Pin, GPIO_PIN_RESET);

    hb_nextSendMs = HAL_GetTick();
    ds_state = DS_TRIG_PULSE;
    ind_phase = IND_PHASE_IDLE;
    ind_nextMs = HAL_GetTick();
}

/**
 * @brief Funkcja wywoływana cyklicznie w pętli głównej — zastępuje działanie RTOS.
 */
void ProcessMultiThreads(void)
{
    DistanceSensor_Update();
    DistanceIndicator_Update();
    HeartBeat_Update();
}
