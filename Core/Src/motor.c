/**
 * @file motor.c
 * @brief Implementacja funkcji sterujących silnikami.
 * @author DeathLock
 * @date 2025-01-30
 */

#include "motor.h"

/**
 * @brief Struktury przechowujące dane silników.
 */
MOTOR motorA;
MOTOR motorB;

/**
 * @brief Inicjalizuje strukturę silnika.
 * @param _motor Wskaźnik na strukturę silnika.
 * @param _encoder Wskaźnik na timer enkodera.
 * @param _axisTimer Kanał timera odpowiadający osi.
 * @param _motorBack Timer silnika do jazdy w tył.
 * @param _motorFront Timer silnika do jazdy w przód.
 */
void Init_Motor(MOTOR* _motor, TIM_HandleTypeDef* _encoder, uint32_t _axisTimer,
                TIM_HandleTypeDef* _motorBack, TIM_HandleTypeDef* _motorFront)
{
    _motor->encoder = _encoder;
    _motor->axisTimer = _axisTimer;

    _motor->motorBack = _motorBack;
    _motor->motorFront = _motorFront;

    _motor->resolution = ENCODER_RESOLUTION;
    _motor->direction = DEFAULT;
}

/**
 * @brief Resetuje ustawienia silnika.
 * @param _motor Wskaźnik na strukturę silnika.
 */
void ResetMotor(MOTOR* _motor)
{
    _motor->set_speed = 0;
    _motor->pulse_count = (int16_t)__HAL_TIM_GET_COUNTER(_motor->encoder);
    __HAL_TIM_SET_COUNTER(_motor->encoder, 0);
    __HAL_TIM_SetCompare(_motor->motorBack, _motor->axisTimer, 0);
    __HAL_TIM_SetCompare(_motor->motorFront, _motor->axisTimer, 0);
}

/**
 * @brief Aktualizuje licznik impulsów enkodera.
 * @param _motor Wskaźnik na strukturę silnika.
 */
void motor_update_count(MOTOR* _motor)
{
    int32_t pulse = (int16_t)__HAL_TIM_GET_COUNTER(_motor->encoder);
    _motor->pulse_count += pulse;
    __HAL_TIM_SET_COUNTER(_motor->encoder, 0);
}

/**
 * @brief Oblicza aktualną prędkość silnika na podstawie impulsów.
 * @param _motor Wskaźnik na strukturę silnika.
 */
void motor_calculate_speed(MOTOR* _motor)
{
    motor_update_count(_motor);

    _motor->measured_speed = (int)((float)(_motor->pulse_count / _motor->resolution) *
                                   (float)(SECOND_IN_MINUTE / TIMER_FREQENCY) /
                                   (float)MOTOR_GEAR);

    int output = pid_update(&_motor->pid_controller, _motor->set_speed, _motor->measured_speed);
    SetMotorSpeed(_motor, output);

    _motor->pulse_count = 0;
}

/**
 * @brief Ustawia docelową prędkość silnika.
 * @param _motor Wskaźnik na strukturę silnika.
 * @param _direction Kierunek obrotu silnika.
 * @param _speed Docelowa prędkość (0-100).
 */
void motor_set_speed(MOTOR* _motor, short _direction, int _speed)
{
    _motor->direction = _direction;
    _motor->set_speed = _speed;
}

/**
 * @brief Ustawia prędkość silnika przez PWM.
 * @param _motor Wskaźnik na strukturę silnika.
 * @param _speed Wartość PWM.
 */
void SetMotorSpeed(MOTOR* _motor, uint16_t _speed)
{
    switch (_motor->direction) {
    case DEFAULT:
        __HAL_TIM_SetCompare(_motor->motorBack, _motor->axisTimer, 0);
        __HAL_TIM_SetCompare(_motor->motorFront, _motor->axisTimer, 0);
        break;
    case FRONT:
        if (_speed >= _motor->motorFront->Instance->ARR)
            _speed = _motor->motorFront->Instance->ARR;
        __HAL_TIM_SetCompare(_motor->motorFront, _motor->axisTimer, _speed);
        __HAL_TIM_SetCompare(_motor->motorBack, _motor->axisTimer, 0);
        break;
    case BACK:
        if (_speed >= _motor->motorBack->Instance->ARR)
            _speed = _motor->motorBack->Instance->ARR;
        __HAL_TIM_SetCompare(_motor->motorBack, _motor->axisTimer, _speed);
        __HAL_TIM_SetCompare(_motor->motorFront, _motor->axisTimer, 0);
        break;
    }
}

/**
 * @brief Inicjalizuje system sterowania silnikami.
 */
void Init_MotorSystem()
{
    Init_Motor(&motorA, &htim8, TIM_CHANNEL_1, &htim5, &htim2);
    Init_Motor(&motorB, &htim3, TIM_CHANNEL_4, &htim5, &htim2);

    pid_init(&(motorA.pid_controller), MOTOR_Kp, MOTOR_Ki, MOTOR_Kd, MOTOR_ANTI_WINDUP);
    pid_init(&(motorB.pid_controller), MOTOR_Kp, MOTOR_Ki, MOTOR_Kd, MOTOR_ANTI_WINDUP);

    ResetMotor(&motorA);
    ResetMotor(&motorB);
}

/**
 * @brief Obsługuje przerwania timera.
 * @param _htim Wskaźnik na strukturę timera.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* _htim)
{
    if (_htim->Instance == TIM6) {
        if (motorA.direction == BACK &&
            motorB.direction == BACK &&
            Distance <= 10) {
            ResetMotor(&motorA);
            ResetMotor(&motorB);
            return;
        }

        motor_calculate_speed(&motorA);
        motor_calculate_speed(&motorB);
    }
}
