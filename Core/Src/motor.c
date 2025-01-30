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
 * @param _axis Kanał timera sterującego osią.
 * @param _front Wskaźnik na timer przedniego silnika.
 * @param _back Wskaźnik na timer tylnego silnika.
 */
void Init_Motor(MOTOR* _motor, TIM_HandleTypeDef* _encoder, uint32_t _axis, TIM_HandleTypeDef* _front, TIM_HandleTypeDef* _back)
{
    _motor->encoder = _encoder;
    _motor->axisTimer = _axis;
    _motor->motorFront = _front;
    _motor->motorBack = _back;

    _motor->resolution = ENCODER_RESOLUTION * TIMER_CONF_BOTH_EDGE_T1T2 * MOTOR_GEAR;

    _motor->pulse_count = 0;
    _motor->measured_speed = 0;
    _motor->set_speed = 0;
    _motor->actual_PWM = 0;
}

/**
 * @brief Oblicza prędkość silnika i aktualizuje wartość PWM.
 * @param _motor Wskaźnik na strukturę silnika.
 */
void motor_calculate_speed(MOTOR* _motor)
{
    motor_update_count(_motor);

    _motor->measured_speed = abs((_motor->pulse_count * TIMER_FREQENCY * SECOND_IN_MINUTE) / _motor->resolution);

    int output = pid_calculate(&(_motor->pid_controller), _motor->set_speed, _motor->measured_speed);

    _motor->actual_PWM += output;

    if (_motor->actual_PWM >= 0)
        SetMotorSpeed(_motor, _motor->actual_PWM);
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
    _motor->pulse_count = (int16_t)__HAL_TIM_GET_COUNTER(_motor->encoder);
    __HAL_TIM_SET_COUNTER(_motor->encoder, 0);
}

/**
 * @brief Ustawia prędkość i kierunek silnika.
 * @param _motor Wskaźnik na strukturę silnika.
 * @param _direction Kierunek obrotów (FRONT/BACK/DEFAULT).
 * @param _set_speed Zadana prędkość obrotowa.
 */
void motor_set_speed(MOTOR* _motor, short _direction, int _set_speed)
{
    if (_set_speed != _motor->set_speed)
        pid_reset(&(_motor->pid_controller));

    _motor->set_speed = _set_speed;
    _motor->direction = _direction;
}

/**
 * @brief Ustawia prędkość silnika na podstawie kierunku obrotów.
 * @param _motor Wskaźnik na strukturę silnika.
 * @param _speed Żądana wartość PWM.
 */
void SetMotorSpeed(MOTOR* _motor, uint16_t _speed)
{
    switch (_motor->direction) {
    case DEFAULT:
        if (_speed >= _motor->motorFront->Instance->ARR)
            _speed = _motor->motorFront->Instance->ARR;
        __HAL_TIM_SetCompare(_motor->motorFront, _motor->axisTimer, _speed);

        if (_speed >= _motor->motorBack->Instance->ARR)
            _speed = _motor->motorBack->Instance->ARR;
        __HAL_TIM_SetCompare(_motor->motorBack, _motor->axisTimer, _speed);
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
    Init_Motor(&motorA, &htim8, TIM_CHANNEL_1, &htim2, &htim5);
    Init_Motor(&motorB, &htim3, TIM_CHANNEL_4, &htim2, &htim5);

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
