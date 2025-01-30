/*
 * pid.h
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#ifndef INC_PID_H_
#define INC_PID_H_

typedef struct
{
	int previous_error;
	int total_error;
	float Kp;
	float Ki;
	float Kd;
	int anti_windup_limit;
} PID;

void pid_init(PID*, float, float, float, int);
void pid_reset(PID*);
int pid_calculate(PID*, int, int);

#endif /* INC_PID_H_ */
