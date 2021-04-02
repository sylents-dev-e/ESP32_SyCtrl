/*
 * adc_driver.h
 *
 *  Created on: 25.03.2020
 *      Author: cleme
 */

#ifndef MAIN_ADC_DRIVER_H_
#define MAIN_ADC_DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "pwm_driver.h"
#include "main.h"

#define DEBUG_PRINT 0 // display data of ADC in serial connection

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   256          //Multisampling

#ifdef __cplusplus
extern "C" {
#endif
void adc_input(void *arg);
#ifdef __cplusplus
}
#endif

#endif /* MAIN_ADC_DRIVER_H_ */
