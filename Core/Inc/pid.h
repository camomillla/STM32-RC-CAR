/*
 * pid.h
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#ifndef INC_PID_H_
#define INC_PID_H_

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float anti_windup;

    float integral;
    float previous_error;
} PID;

void pid_init(PID* pid, float Kp, float Ki, float Kd, float anti_windup);
int pid_update(PID* pid, float setpoint, float measured_value);

#endif /* INC_PID_H_ */
