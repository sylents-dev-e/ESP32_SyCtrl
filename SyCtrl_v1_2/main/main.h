/*
 * main.h
 *
 *  Created on: 26.03.2020
 *      Author: cleme
 */

#ifndef MAIN_MAIN_H_
#define MAIN_MAIN_H_

// LIBRARIES
#include <sys/param.h>
#include <stdio.h>
#include <strings.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "sdkconfig.h"
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
#include "wifi_driver.h"
#include "spi_driver.h"
#include "spi_lcd_ILI9341.h"
#include "data_ILI9341.h"
#include "http_webserver.h"
#include "lvgl/lvgl.h"
#include "lvgl_driver.h"
#include "cJSON.h"
#include "can_driver.h"
#include "data_logging.h"

#ifdef __cplusplus
extern "C" {
#endif
void app_main(void);
bool driver_cfg(void);
bool init_network(void);
bool intertask_com(void);
bool task_create(void);
#ifdef __cplusplus
}
#endif

// Queues
QueueHandle_t xQueue;
QueueHandle_t queue_button;
QueueHandle_t queue_to_display_dig;
QueueHandle_t queue_to_display_adc;
QueueHandle_t queue_to_ili9341;
QueueHandle_t queue_http;
QueueHandle_t queue_to_json_to_app;
QueueHandle_t queue_to_json_to_app_adc;


// Semaphore
SemaphoreHandle_t UpButtonSemaphore;
SemaphoreHandle_t DownButtonSemaphore;
SemaphoreHandle_t CountSemaphore;
SemaphoreHandle_t GuiSemaphore;
SemaphoreHandle_t ButtonThrottleAppSemaphore;
SemaphoreHandle_t ButtonSteeringAppSemaphore;
SemaphoreHandle_t twai_tx_sem;
SemaphoreHandle_t can_rx_sem;
SemaphoreHandle_t can_ctrl_sem;
SemaphoreHandle_t can_done_sem;

// task handlers
TaskHandle_t xTaskToNotify;

/* enable throttle PWM */
#define PWM_THROTTLE			0

// SW version to display
#define SW_VERSION 				"#C0C0C0 SW Version SyCtrl v1.2"

// WDT timeout
#define TWDT_TIMEOUT_S			10
#define TASK_STACK_SIZE 		2048

// DEBUG Messages enable
#define DEBUG_PRINT_GLOBAL 		0
#define DEBUG_PRINT_BUTTON 		0
#define DEBUG_PRINT_ADC 		0
#define DEBUG_TASK_STACK		0

#define WDT_PWM 				CONFIG_ESP_WDT_PWM

#define SELECTED_FONT_SIZE 		28

// app specific
#define LEFT_FROM_APP 			11
#define RIGHT_FROM_APP 			12

// 12 BIT / 2 ADC EMULATION
#define INIT_APP_STEERING_POS 	2048

// Core ID
#define CORE_ID_0				0
#define CORE_ID_1				1

// checking function for watchdog timer
#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                printf("TWDT ERROR\n");                                \
                abort();                                               \
            }                                                          \
})

#endif /* MAIN_MAIN_H_ */
