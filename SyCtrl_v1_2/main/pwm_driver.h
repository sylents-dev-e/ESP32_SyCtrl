/*
 * pwm.h
 *
 *  Created on: 24.03.2020
 *      Author: cleme
 */

#ifndef MAIN_PWM_DRIVER_H_
#define MAIN_PWM_DRIVER_H_

#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "main.h"
#include "adc_driver.h"

#ifdef __cplusplus
extern "C" {
#endif
void pwm_throttle_task(void *arg);
void pwm_steering_task(void *arg);
void forward_set_duty(float selected_duty, int8_t mode);
bool pwm_gpio_init();
#ifdef __cplusplus
}
#endif

/* pin setting */
#define GPIO_PWM0_THROTTLE_OUT 	4 							/* set GPIO 04 */
#define GPIO_PWM1_STEERING_OUT 	16   						/* Set GPIO 16 */

/* PMW paramters */
#define PWM_MIN 				4.25 						/* 850탎 */
#define PWM_MAX 				11.00 						/* 2200탎 */
#define PWM_STEPS 				4
#define PWM_INC 				(float)((PWM_MAX-PWM_START_CURRENT)/PWM_STEPS)
#define PWM_START_CURRENT 		4.60

/* coefficient for calc duty (pwm unit) */
#define PWM_COEFF 				200

/* limits for steering center position stabilisation */
#define LOW_LIMIT_STEERING 		1800
#define HIGH_LIMIT_STEERING 	1920

/* Duty Cycles THROTTLE */
#if PWM_THROTTLE
#define MODE1 					4.25 //850탎
#define MODE2 					4.60 //920탎
#define MODE3 					7.00 //1400탎
#define MODE4 					9.00 //1800탎
#define MODE5 					11.00 //2200탎
#endif

/* Duty Cycles STEERING */
#define MODE_RIGHT 				10.00 //2000탎 FULL RIGHT
#define MODE_LEFT 				5.00 //1000탎 FULL LEFT
#define MODE_CENTER 			7.50 //1500탎 CENTER
#define STEERING_CALC 			((MODE_RIGHT-MODE_LEFT)/4095)

#endif /* MAIN_PWM_DRIVER_H_ */
