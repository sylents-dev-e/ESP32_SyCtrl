/*
 * button_driver.c
 *
 *  Created on: 27.03.2020
 *      Author: cleme
 */

#include "button_driver.h"

/**
 * @fn void gpio_isr_handler(void*)
 * @brief interrupt service routine for emergency button, sends pin number to pwm unit
 * @param arg set in the button config, the specific pin number
 */

static void IRAM_ATTR gpio_isr_handler(void *arg) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t gpio_num = (uint32_t) arg;

	/* send direction to PWM if button is pressed.
	 * BUGFIX: using xQueueOverwriteFromISR avoids unwanted
	 * 		   behavior on the backside of the queue.
	 */
	xQueueOverwriteFromISR(queue_button, &gpio_num, NULL);
	gpio_num = 0;

	// releasing semaphore to start task operation in pwm
	xSemaphoreGiveFromISR(CountSemaphore, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken == pdTRUE) {
		/* Actual macro used here is port speciic. */
		portYIELD_FROM_ISR();
	}
}

/**
 * @fn bool button_isr_config(uint32_t)
 * @brief ISR button configuration, will be set to ISR
 * 		  mode in every case.
 *
 * @param pin_btn
 * @return true on succes
 */

bool button_emergency_isr_config(uint32_t pin_btn) {
	//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	// config params
	gpio_config_t io_conf;
	//interrupt of neg edge
	io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
	io_conf.pin_bit_mask = 1ULL << pin_btn;
	//set as input mode
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

	//config pin
	if (gpio_config(&io_conf) != ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}

	//GPIO interrupt wakeup on that pin, rising edge
	if (gpio_wakeup_enable(pin_btn, GPIO_INTR_LOW_LEVEL) != ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}

	//enable interrupt on that pin
	if (gpio_intr_enable(pin_btn) != ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}

	ESP_ERROR_CHECK(
			gpio_isr_handler_add(pin_btn, gpio_isr_handler, (void*) EMERGENCY));

	return true;
}

/**
 * @fn bool button_config(uint32_t)
 * @brief initialization and config of the peripherie: GPIO
 * 		  can be modified eigther to ISR mode or "normal"
 * 		  digital pin-read mode
 * @param pin_btn selected GPIO pin
 * @return true on success
 */

bool button_config(uint32_t pin_btn) {

	// config params
	gpio_config_t io_conf;
#if BUTTON_ISR_MODE
	io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
#else
	io_conf.intr_type = GPIO_INTR_DISABLE;
#endif
	io_conf.pin_bit_mask = 1ULL << pin_btn;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

	// config pin
	if (gpio_config(&io_conf) != ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}
#if BUTTON_ISR_MODE
	//GPIO interrupt wakeup on that pin, rising edge
	if (gpio_wakeup_enable(pin_btn, GPIO_INTR_LOW_LEVEL) != ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}

	//enable interrupt on that pin
	if (gpio_intr_enable(pin_btn) != ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}

	//hook specific isr handler for gpio pin
	switch (pin_btn) {
	case UP:
		if (gpio_isr_handler_add(pin_btn, gpio_isr_handler,
				(void*) UP) == ESP_OK) {
			return true;
		} else
			return false;
	case DOWN:
		if (gpio_isr_handler_add(pin_btn, gpio_isr_handler,
				(void*) DOWN) == ESP_OK) {
			return true;
		} else
			return false;
	default:
		return false;
	}
#else
	return true;
#endif
}

/**
 * @fn void button_task(void*)
 * @brief button task, initializes buttons at startup
 *		  perodically checks non-ISR buttons for their
 *		  state.
 *
 * @param arg
 */

void button_task(void *arg) {

#if DEBUG_TASK_STACK
	UBaseType_t uxHighWaterMark;
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
#endif

	// WTD init for that task
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);
	uint32_t gpio_num = 0;

	// init buttons GPIO
	configASSERT(button_emergency_isr_config(EMERGENCY));
	configASSERT(button_config(UP));
	configASSERT(button_config(DOWN));

	while (1) {
		// give response to WDT
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);

// non-ISR button method
#if !BUTTON_ISR_MODE
		if ((gpio_get_level(DOWN)) == 0) {

			gpio_num = DOWN;
			if ( xSemaphoreTake(ButtonThrottleAppSemaphore,
					(TickType_t ) 10) == pdTRUE) {
				xQueueOverwrite(queue_button, (void* ) &gpio_num);
				xSemaphoreGive(ButtonThrottleAppSemaphore);
			} else {
				__asm__ volatile ( "NOP" );
			}
			xSemaphoreGive(CountSemaphore);
		} else if ((gpio_get_level(UP)) == 0) {

			gpio_num = UP;
			if ( xSemaphoreTake(ButtonThrottleAppSemaphore,
					(TickType_t ) 10) == pdTRUE) {

				xQueueOverwrite(queue_button, (void* ) &gpio_num);
				xSemaphoreGive(ButtonThrottleAppSemaphore);
			} else {
				__asm__ volatile ( "NOP" );
			}
		} else {
			gpio_num = 0;
			xQueueReset(queue_button);
		}
		xSemaphoreGive(CountSemaphore);
#endif

		// minimum 100ms to fit with button task
		vTaskDelay(100/ portTICK_PERIOD_MS);

#if DEBUG_TASK_STACK
			uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
			ets_printf("%d KB %s\n", uxHighWaterMark, "button_task");
#endif

	}
	vTaskDelete(NULL);
}

