/*
 * modules.h
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#ifndef INC_MODULES_H_
#define INC_MODULES_H_

#include "tim.h"
#include "motor.h"
#include "usart.h"

extern uint8_t rxData[1];
extern char rxBuff[64];
extern uint8_t rxIdx;

extern uint32_t pMillis;
extern uint32_t Value1;
extern uint32_t Value2;
extern uint32_t Distance;

extern int hornOn;
extern int indicationOverride;
extern int engineOn;

void Set_PWM_Frequency(uint32_t);
void Command_Init(void);
void Command_Engine(void);
void Command_Lights(void);
void Command_Horn(void);
void Command_ABS(void);
void Command_Motors(uint8_t*);
void ProcessCommand(uint8_t*);

#endif /* INC_MODULES_H_ */
