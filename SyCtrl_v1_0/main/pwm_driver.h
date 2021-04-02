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
bool pwm_generation();
void pwm_throttle_task(void *arg);
void pwm_steering_task(void *arg);
bool init_motors(void);
void forward_set_duty(float selected_duty);
#ifdef __cplusplus
}
#endif

#define MCPWM_EN_CARRIER 0   //Make this 1 to test carrier submodule of mcpwm, set high frequency carrier parameters
#define MCPWM_EN_DEADTIME 0  //Make this 1 to test deadtime submodule of mcpwm, set deadtime value and deadtime mode
#define MCPWM_EN_FAULT 1     //Make this 1 to test fault submodule of mcpwm, set action on MCPWM signal on fault occurence like overcurrent, overvoltage, etc
#define MCPWM_EN_SYNC 1      //Make this 1 to test sync submodule of mcpwm, sync timer signals
#define MCPWM_EN_CAPTURE 0   //Make this 1 to test capture submodule of mcpwm, measure time between rising/falling edge of captured signal
#define MCPWM_GPIO_INIT 1    //select which function to use to initialize gpio signals
#define CAP_SIG_NUM 3   //Three capture signals

#define CAP0_INT_EN BIT(27)  //Capture 0 interrupt bit
#define CAP1_INT_EN BIT(28)  //Capture 1 interrupt bit
#define CAP2_INT_EN BIT(29)  //Capture 2 interrupt bit

#define GPIO_PWM0_THROTTLE_OUT 19   //Set GPIO 19
#define GPIO_PWM0_STEERING_OUT 18   //Set GPIO 18

#define GPIO_SYNC0_IN   2   //Set GPIO 02 as SYNC0
#define GPIO_SYNC1_IN   4   //Set GPIO 04 as SYNC1
#define GPIO_SYNC2_IN   5   //Set GPIO 05 as SYNC2
#define GPIO_FAULT0_IN 32   //Set GPIO 32 as FAULT0
#define GPIO_FAULT1_IN 34   //Set GPIO 34 as FAULT1
#define GPIO_FAULT2_IN 34   //Set GPIO 34 as FAULT2

#define PWM_MIN 4.25 //850탎
#define PWM_MAX 11.00 //2200탎
#define PWM_STEPS 4 //
#define PWM_INC (float)((PWM_MAX-PWM_START_CURRENT)/PWM_STEPS)

// coefficient for calc duty (pwm unit)
#define PWM_COEFF 200

#define PWM_START_CURRENT 4.60

//TODO: berechnung zur laufzeit
// Duty Cycles THROTTLE
#define MODE1 4.25 //850탎
#define MODE2 4.60 //920탎
#define MODE3 7.00 //1400탎
#define MODE4 9.00 //1800탎
#define MODE5 11.00 //2200탎

// Duty Cycles STEERING

#define MODE_RIGHT 10.00 //2000탎 FULL RIGHT
#define MODE_LEFT 5.00 //1000탎 FULL LEFT
#define MODE_CENTER 7.50 //1500탎 CENTER
#define STEERING_CALC ((MODE_RIGHT-MODE_LEFT)/4095)

#endif /* MAIN_PWM_DRIVER_H_ */
