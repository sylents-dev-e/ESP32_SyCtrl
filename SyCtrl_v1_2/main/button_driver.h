/*
 * button_driver.h
 *
 *  Created on: 27.03.2020
 *      Author: cleme
 */

#ifndef MAIN_BUTTON_DRIVER_H_
#define MAIN_BUTTON_DRIVER_H_

#include "main.h"

#define ESP_INTR_FLAG_DEFAULT 0

#define EMERGENCY 33
#define UP 35
#define DOWN 34

#define BUTTON_ISR_MODE 0

#define DEBOUNCE_DELAY  (50 / portTICK_RATE_MS)

#ifdef __cplusplus
extern "C" {
#endif
// has to be 32bit otherwise ISR would not work
bool button_config(uint32_t pin_btn);
void button_task (void *arg);
bool button_emergency_isr_config(uint32_t pin_btn);
#ifdef __cplusplus
}
#endif

#endif /* MAIN_BUTTON_DRIVER_H_ */
