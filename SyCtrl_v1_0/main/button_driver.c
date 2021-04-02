/*
 * button_driver.c
 *
 *  Created on: 27.03.2020
 *      Author: cleme
 */

#include "button_driver.h"

/**
 * @fn void gpio_isr_handler(void*)
 * @brief interrupt service routine for buttons, sends pin number to pwm unit
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
 * @fn bool button_config(uint32_t)
 * @brief initialization and config of the peripherie: GPIO
 * @param pin_btn selected GPIO pin
 * @return true on success
 */

bool button_config(uint32_t pin_btn) {

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

	//hook specific isr handler for gpio pin
	switch (pin_btn) {
	case LEFT:
		if (gpio_isr_handler_add(pin_btn, gpio_isr_handler,
				(void*) LEFT) == ESP_OK) {
			return true;
		} else
			return false;
	case RIGHT:
		if (gpio_isr_handler_add(pin_btn, gpio_isr_handler,
				(void*) RIGHT) == ESP_OK) {
			return true;
		} else
			return false;
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

}

