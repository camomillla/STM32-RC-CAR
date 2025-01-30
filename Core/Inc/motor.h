/*
 * motor.h
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "tim.h"
#include "pid.h"
#include "modules.h"

#define MOTOR_Kp					3
#define MOTOR_Ki					0.05
#define MOTOR_Kd					0.05
#define MOTOR_ANTI_WINDUP			1

#define ENCODER_RESOLUTION			3
#define TIMER_CONF_BOTH_EDGE_T1T2	4
#define MOTOR_GEAR					50

#define	TIMER_FREQENCY				20
#define	SECOND_IN_MINUTE			60

typedef struct
{
	TIM_HandleTypeDef *encoder;

	TIM_HandleTypeDef *motorBack;
	TIM_HandleTypeDef *motorFront;
	uint32_t axisTimer;
	short direction;

	uint16_t resolution;

	int pulse_count;
	int measured_speed;
	int set_speed;

	int actual_PWM;

	PID pid_controller;
} MOTOR;

enum DIRECTION {
	DEFAULT, FRONT, BACK
};

extern MOTOR motorA;
extern MOTOR motorB;

void Init_Motor(MOTOR*, TIM_HandleTypeDef*, uint32_t, TIM_HandleTypeDef*, TIM_HandleTypeDef*);
void motor_calculate_speed(MOTOR*);
void ResetMotor(MOTOR*);
void motor_update_count(MOTOR*);
void motor_set_speed(MOTOR*, short, int);
void MotorABS(MOTOR*);
void SetMotorSpeed(MOTOR*, uint16_t);
void Init_MotorSystem(void);

#endif /* INC_MOTOR_H_ */
