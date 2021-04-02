/*
 * main.h
 *
 *  Created on: 26.03.2020
 *      Author: cleme
 */

#ifndef MAIN_MAIN_H_
#define MAIN_MAIN_H_

#include <stdio.h>
#include <strings.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "pwm_driver.h"
#include "adc_driver.h"
#include "button_driver.h"
#include "esp_task_wdt.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "i2c_driver.h"
#include "display_driver.h"
#include "spi_driver.h"
#include "spi_lcd_ILI9341.h"

#ifdef __cplusplus
extern "C" {
#endif
void app_main(void);
bool driver_cfg(void);
bool queue_semaphore_create(void);
#ifdef __cplusplus
}
#endif

// Global variables
QueueHandle_t xQueue;
QueueHandle_t queue_button;
QueueHandle_t queue_to_display_dig;
QueueHandle_t queue_to_display_adc;


// Semaphore
xSemaphoreHandle UpButtonSemaphore;
xSemaphoreHandle DownButtonSemaphore;
xSemaphoreHandle CountSemaphore;

TaskHandle_t xTaskToNotify;

// WDT timeout
#define TWDT_TIMEOUT_S	3
#define TASK_STACK_SIZE 2048

// checking function for watchdog timer
#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                printf("TWDT ERROR\n");                                \
                abort();                                               \
            }                                                          \
})

#endif /* MAIN_MAIN_H_ */
