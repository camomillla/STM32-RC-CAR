/*
 * pid.c
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#include "pid.h"

/**
 * @brief Inicjalizacja struktury PID
 * @details Ustawia początkowe wartości dla członów regulacji oraz limit anty-windup
 * @param pid_data Uchwyt struktury PID
 * @param _kp_init Wartość nastawy kp
 * @param _ki_init Wartość nastawy ki
 * @param _kd_init Wartość nastawy kd
 * @param _anti_windup_limit_init Wartość parametru AntiWindup
 */
void pid_init(PID* _pid_data, float _kp_init, float _ki_init, float _kd_init, int _anti_windup_limit_init)
{
	_pid_data->previous_error = 0;
	_pid_data->total_error = 0;

	_pid_data->Kp = _kp_init;
	_pid_data->Ki = _ki_init;
	_pid_data->Kd = _kd_init;

	_pid_data->anti_windup_limit = _anti_windup_limit_init;
}

/**
 * @brief Resetowanie wartości błędu w regulatorze PID
 * @param pid_data Uchwyt struktury PID
 */
void pid_reset(PID *_pid_data)
{
	_pid_data->total_error = 0;
	_pid_data->previous_error = 0;
}

/**
 * @brief Obliczanie wartości sterującej regulatora PID
 * @param pid_data Uchwyt struktury PID
 * @param _setpoint Wartość zadana
 * @param _process_variable Wartość zmierzona
 */
int pid_calculate(PID *_pid_data, int _setpoint, int _process_variable)
{
	int error;
	float p_term, i_term, d_term;

	error = _setpoint - _process_variable;
	_pid_data->total_error += error;

	p_term = (float)(_pid_data->Kp * error);
	i_term = (float)(_pid_data->Ki * _pid_data->total_error);
	d_term = (float)(_pid_data->Kd * (error - _pid_data->previous_error));

	if(i_term >= _pid_data->anti_windup_limit) i_term = _pid_data->anti_windup_limit;
	else if(i_term <= -_pid_data->anti_windup_limit) i_term = -_pid_data->anti_windup_limit;

	_pid_data->previous_error = error;

	return (int)(p_term + i_term + d_term);
}
