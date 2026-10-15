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

/* Diagnostyka odbioru USART2 widoczna w debuggerze. */
extern volatile uint32_t uart2_rx_bytes;
extern volatile uint32_t uart2_rx_commands;
extern volatile uint32_t uart2_rx_processed;
extern volatile uint32_t uart2_rx_errors;
extern volatile uint32_t uart2_rx_overruns;
extern volatile uint32_t uart2_rx_dropped;
extern volatile uint32_t uart2_rx_rearm_failures;
extern volatile uint32_t uart2_last_error;
extern volatile uint32_t uart2_last_valid_tick;
extern volatile uint32_t uart2_link_timeout_count;

void UART2_ProcessPendingCommands(void);
void UART2_EnsureReception(void);
void Set_PWM_Frequency(uint32_t);
void Command_Init(void);
void Command_Ping(void);
void Command_Engine(void);
void Command_Lights(void);
void Command_Horn(void);
void Command_ABS(void);
void Command_Motors(uint8_t*);
void ProcessCommand(uint8_t*);

#endif /* INC_MODULES_H_ */
