/*
 * thread.h
 *
 *  Created on: Jan 30, 2025
 *      Author: DeathLock
 */

#ifndef INC_THREAD_H_
#define INC_THREAD_H_

#include "main.h"
#include "modules.h"
#include "tim.h"

/**
 * @brief Inicjalizuje logikę wątków (wersja bez RTOS).
 * @details Funkcja zachowuje nazwę z wersji RTOS, ale nie tworzy wątków —
 *          inicjalizuje stan maszyn i uruchamia wymagane peryferia.
 */
void StartMultiThreads(void);

/**
 * @brief Uruchamia logikę dawnych wątków w trybie kooperacyjnym.
 * @details Należy wywoływać możliwie często z pętli głównej.
 */
void ProcessMultiThreads(void);

#endif /* INC_THREAD_H_ */
