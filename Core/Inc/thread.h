/*
 * thread.h
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#ifndef INC_THREAD_H_
#define INC_THREAD_H_

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "tim.h"
#include "modules.h"

void ProcessHeartBeat(void*);
void TESTSCENARIO(void*);
void DistanceSensor(void*);
void DistanceIndicator(void*);

void StartMultiThreads(void);

extern osThreadId_t heartBeatHandle;
extern osThreadId_t testScenario;
extern osThreadId_t distanceSensorHandle;
extern osThreadId_t distanceIndicatorHandle;

#endif /* INC_THREAD_H_ */
