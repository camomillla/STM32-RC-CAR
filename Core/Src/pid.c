/*
 * pid.c
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#include "pid.h"

/**
 * @brief Inicjalizuje regulator PID.
 * @param pid Wskaźnik na strukturę PID.
 * @param Kp Wzmocnienie proporcjonalne.
 * @param Ki Wzmocnienie całkujące.
 * @param Kd Wzmocnienie różniczkujące.
 * @param anti_windup Wartość ograniczenia całki.
 */
void pid_init(PID* pid, float Kp, float Ki, float Kd, float anti_windup)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->anti_windup = anti_windup;

    pid->integral = 0.0f;
    pid->previous_error = 0.0f;
}

/**
 * @brief Oblicza wyjście regulatora PID.
 * @param pid Wskaźnik na strukturę PID.
 * @param setpoint Wartość zadana.
 * @param measured_value Wartość zmierzona.
 * @return Wartość wyjściowa regulatora.
 */
int pid_update(PID* pid, float setpoint, float measured_value)
{
    float error = setpoint - measured_value;

    pid->integral += error;
    if (pid->integral > pid->anti_windup) pid->integral = pid->anti_windup;
    if (pid->integral < -pid->anti_windup) pid->integral = -pid->anti_windup;

    float derivative = error - pid->previous_error;
    pid->previous_error = error;

    float output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;

    if (output < 0.0f) output = 0.0f;
    return (int)output;
}
