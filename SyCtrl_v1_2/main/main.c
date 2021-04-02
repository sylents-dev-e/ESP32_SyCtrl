/**
 * @file main.h
 *
 *  Created on: 26.03.2020
 *      Author: cleme
 */

#include "main.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

static TaskHandle_t task_handles[portNUM_PROCESSORS];

/**
 * @fn void app_main(void)
 *
 * @brief main app to start init-functions and get the OS running
 */

void app_main(void) {

	// initialization non-volatile storage
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// initialize or reinitialize TWDT
	CHECK_ERROR_CODE(esp_task_wdt_init(TWDT_TIMEOUT_S, false), ESP_OK);

#ifndef CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU0
    esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
#endif
#ifndef CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1
    esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
#endif

	// JSON object creation
	json_operations();

	// init server and wifi
	configASSERT(init_network());

	// create intertask communication
	configASSERT(intertask_com());

	// init drivers
	configASSERT(driver_cfg());

	vTaskDelay(100 / portTICK_PERIOD_MS);

	// create tasks
	configASSERT(task_create());

	while (1) {
		/* WTD timing config to avoid the following error:
		 * "Task watchdog got triggered. The following tasks did not reset the watchdog in time: - IDLE (CPU 0)
		 */

		TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
		TIMERG0.wdt_feed = 1;
		TIMERG0.wdt_wprotect = 0;

#if DEBUG_PRINT_GLOBAL
		// just to check if scheduler is running
		printf("Tick rate %d Hz\n", CONFIG_FREERTOS_HZ);
		printf("scheduler %d\n", xTaskGetSchedulerState());
		vTaskDelay(1000/ portTICK_PERIOD_MS);
#endif
	}
}

bool driver_cfg(void) {

	// init pwm unit
	configASSERT(pwm_gpio_init());

	// init CAN
	configASSERT(can_initialization());

	return true;
}

/**
 * @fn bool intertask_com(void)
 *
 * @brief creating and setting different interprocess/task communications
 *
 * @return true 	on success
 * 		   false 	on error
 */
bool intertask_com(void) {

	//message queue for digital button com
	queue_button = xQueueCreate(1, sizeof(int32_t));
	configASSERT(queue_button);

	//message queue for adc joystick
	xQueue = xQueueCreate(1, sizeof(uint32_t));
	configASSERT(xQueue);

	//message queue for adc joystick to display
	queue_to_display_adc = xQueueCreate(1, sizeof(uint32_t));
	configASSERT(queue_to_display_adc);

	//message queue for ili9341 display
	queue_to_ili9341 = xQueueCreate(1, sizeof(int32_t));
	configASSERT(queue_to_ili9341);

	// semaphore for GUI
	GuiSemaphore = xSemaphoreCreateMutex();
	configASSERT(GuiSemaphore);

	// semaphore for buttons
	vSemaphoreCreateBinary(CountSemaphore);
	configASSERT(CountSemaphore);

	// semaphore for up/down
	ButtonThrottleAppSemaphore = xSemaphoreCreateMutex();
	configASSERT(ButtonThrottleAppSemaphore);

	// semaphore for left/right
	ButtonSteeringAppSemaphore = xSemaphoreCreateMutex();
	configASSERT(ButtonSteeringAppSemaphore);

	// queue for http to display
	queue_http = xQueueCreate(1, sizeof(int32_t));
	configASSERT(queue_http);

	// queue backwards from pwm to json string to smartphone
	queue_to_json_to_app = xQueueCreate(1, sizeof(int32_t));
	configASSERT(queue_to_json_to_app);

	// queue backwards from pwm to json string to smartphone
	queue_to_json_to_app_adc = xQueueCreate(1, sizeof(uint32_t));
	configASSERT(queue_to_json_to_app_adc);

	// CAN related semaphores
	twai_tx_sem = xSemaphoreCreateBinary();
	configASSERT(twai_tx_sem);
	can_rx_sem = xSemaphoreCreateBinary();
	configASSERT(can_rx_sem);
	can_ctrl_sem = xSemaphoreCreateBinary();
	configASSERT(can_ctrl_sem);
	can_done_sem = xSemaphoreCreateBinary();
	configASSERT(can_done_sem);

	return true;
}

/**
 * @fn bool init_network(void)
 *
 * @brief setting the Wifi init function
 *
 * @return true 	on success
 * 		   false 	on error
 */
bool init_network(void) {

	ESP_ERROR_CHECK(init_wifi());

	return true;
}

/**
 * @fn bool task_create(void)
 *
 * @brief tasks creation
 *
 * @return true 	on success
 * 		   false 	on error
 */
bool task_create(void) {

	// ADC task
	if (xTaskCreatePinnedToCore(adc_input, "adc_input", TASK_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 3), &task_handles[1], CORE_ID_0) != pdPASS) {
		return false;
	}

	// button task
	else if (xTaskCreatePinnedToCore(button_task, "button_task",
			TASK_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2), &task_handles[1],
			CORE_ID_0) != pdPASS) {
		return false;
	}

	// gui Task
	else if (xTaskCreatePinnedToCore(guiTask, "guiTask", TASK_STACK_SIZE * 4,
			NULL, (tskIDLE_PRIORITY + 1), &task_handles[1], CORE_ID_1) != pdPASS) {
		return false;
	}

#if PWM_THROTTLE
	// pwm_throttle task
	else if (xTaskCreatePinnedToCore(pwm_throttle_task, "pwm_throttle_task",
			TASK_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 5), &task_handles[1],
			CORE_ID_0) != pdPASS) {
		return false;
	}
#endif

	// pwm_steering task
	else if (xTaskCreatePinnedToCore(pwm_steering_task, "pwm_steering_task",
			TASK_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 4), &task_handles[1],
			CORE_ID_0) != pdPASS) {
		return false;
	}

	// can_ctrl task
	else if(xTaskCreatePinnedToCore(can_ctrl_task, "CAN_ctrl", TASK_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 6), &task_handles[1], tskNO_AFFINITY) != pdPASS){
		return false;
	}

	// can_rx task
	else if((xTaskCreatePinnedToCore(can_rx_task, "CAN_rx", TASK_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 8), &task_handles[1], tskNO_AFFINITY)) != pdPASS){
		return false;
	}

	// can_tx task
	else if((xTaskCreatePinnedToCore(can_tx_task, "CAN_tx", TASK_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 7), &task_handles[1], tskNO_AFFINITY)) != pdPASS){
		return false;
	}

	/* SD-card (SPI) task */
	else if((xTaskCreatePinnedToCore(data_logging_task, "data_logging_task", TASK_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 9), &task_handles[1], tskNO_AFFINITY)) != pdPASS){
		return false;
	}

	// success creating tasks
	else {
		return true;
	}
}

