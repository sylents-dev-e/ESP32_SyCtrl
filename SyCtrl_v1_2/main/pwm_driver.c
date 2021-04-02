/*
 * pwm.c
 *
 *  Created on: 24.03.2020
 *      Author: cleme
 *
 */

#include "pwm_driver.h"

/**
 * @fn bool pwm_gpio_init()
 * @brief initializing the PWM GPIO and setting default values
 * @return true on success
 */

bool pwm_gpio_init() {

	ets_printf("initializing mcpwm gpio...\n");

	/*
	 * setting default
	 * no fault detection, no sync
	 * no capture unit (hall sensor)
	 */

	/* steering PWM */
	mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, GPIO_PWM1_STEERING_OUT);
	mcpwm_config_t init_pwm_steering;
	init_pwm_steering.frequency = 50;						/* Hz */
	init_pwm_steering.cmpr_a = MODE_CENTER;					/* init duty */
	init_pwm_steering.counter_mode = MCPWM_UP_COUNTER;
	init_pwm_steering.duty_mode = MCPWM_DUTY_MODE_0; 		/* active HIGH */
	mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &init_pwm_steering);
	mcpwm_start(MCPWM_UNIT_1, MCPWM_TIMER_1);

	/* throttle PWM */
#if PWM_THROTTLE

	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0_THROTTLE_OUT);
	mcpwm_config_t init_pwm_throttle;
	init_pwm_throttle.frequency = 50;						/* Hz */
	init_pwm_throttle.cmpr_a = MODE1;						/* init duty */
	init_pwm_throttle.counter_mode = MCPWM_UP_COUNTER;
	init_pwm_throttle.duty_mode = MCPWM_DUTY_MODE_0; 		/* active HIGH */
	mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &init_pwm_throttle);
	mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
#endif

	return true;
}

/**
 * @fn void pwm_throttle_task(void*)
 * @brief PWM creation through a duty cycle set by button ISR
 * @param arg not used
 */

void pwm_throttle_task(void *arg) {

#if DEBUG_TASK_STACK
	UBaseType_t uxHighWaterMark;
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
#endif

#if WDT_PWM
	// WTD init for that task
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);
#endif

	while (1) {

#if WDT_PWM
		// give response to WDT
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
#endif

#if PWM_THROTTLE

		uint32_t c = 0;
		int32_t mode_cnt = 0;

		// receiving sempahore and queue data from button ISR
		// TODO: maybe better with portMAX_DELAY but then WDT will alert
		if ((xSemaphoreTake( CountSemaphore, portMAX_DELAY ) == pdTRUE)
				&& (xQueueReceive(queue_button, &c, portMAX_DELAY) == pdPASS)) {

			if (c == EMERGENCY) {
				// emergency stop
				forward_set_duty(MODE1, mode_cnt);
				// reset count variable
				mode_cnt = 0;
			} else {
				// counter variable to figure out which duty mode should be set
				if ((c == UP) && (mode_cnt < 4)) {
					mode_cnt++;
					c = 0;
				} else if ((c == DOWN) && (mode_cnt > -1)) {
					mode_cnt--;
					c = 0;
				} else {
					;
				}

				/* setting pwm-cycle depending on counter variable *mode_cnt*
				 * switch case for faster response on several cases instead of
				 * if/else if
				 */
				switch (mode_cnt) {
				case -1:
#if	DEBUG_PRINT_BUTTON
					printf("count: %d\t not implemented\n", mode_cnt);
					break; //not yet implemented
#endif
				case 0:
					forward_set_duty(MODE1, mode_cnt);
					break;
				case 1:
					forward_set_duty(PWM_START_CURRENT, mode_cnt);
					break;
				case 2:
					forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT,
							mode_cnt);
					break;
				case 3:
					forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT,
							mode_cnt);
					break;
				case 4:
					forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT,
							mode_cnt);
					break;
				default:
					forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT,
							mode_cnt);
					break;
				}
			}

			// send gear data to display
			xQueueSend(queue_to_ili9341, &mode_cnt, (TickType_t ) 10);

			// Send data to http server --> JSON to App
			xQueueSend(queue_to_json_to_app, &mode_cnt, (TickType_t ) 10);

			/* Wait a short time then clear any pending button pushes as a crude
			 method of debouncing the switch.  xSemaphoreTake() uses a block time of
			 zero this time so it returns immediately rather than waiting for the
			 interrupt to occur. */

			vTaskDelay(100/ portTICK_PERIOD_MS);

			xSemaphoreTake(CountSemaphore, 0);

		}
#endif

#if DEBUG_TASK_STACK
			uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
			ets_printf("%d KB %s\n", uxHighWaterMark, "pwm_throtte_task");
#endif
	}

	vTaskDelete(NULL);
}

/**
 * @fn void pwm_steering_task(void*)
 * @brief receiving queue data from ADC Joystick to
 * 		  control the steering servo motor
 * @param arg not used
 */
void pwm_steering_task(void *arg) {

#if DEBUG_TASK_STACK
	UBaseType_t uxHighWaterMark;
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
#endif

#if WDT_PWM
	// WTD init for that task
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);
#endif

	// variables
	uint32_t adc_in = 0;

	while (1) {
		// give response to WDT
#if WDT_PWM
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
#endif
		// receive message queue sent from ADC
		if ( xQueueReceive( xQueue, &adc_in,( TickType_t ) 10 ) == pdPASS) {
//			if ((LOW_LIMIT_STEERING < adc_in) && (adc_in < HIGH_LIMIT_STEERING)) {
//				mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A,
//				MODE_CENTER);
//			} else {
			mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A,
					(((float) adc_in * STEERING_CALC) + 5.0));
//			}
			// Send data to display
			xQueueSend(queue_to_display_adc, &adc_in, (TickType_t ) 10);

			// Send data to http server --> JSON to App
			xQueueSend(queue_to_json_to_app_adc, &adc_in, (TickType_t ) 10);

#if DEBUG_PRINT_ADC
		ets_printf("adc_in: %d\n", adc_in);
#endif
		}

		vTaskDelay(100/ portTICK_PERIOD_MS);

#if DEBUG_TASK_STACK
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
		ets_printf("%d KB %s\n", uxHighWaterMark, "pwm_steering_task");
#endif

	}
	vTaskDelete(NULL);
}

/**
 * @fn void forward_set_duty(float, uint8_t)
 * @brief set the selected duty cycle for the pwm
 *
 * @param selected_duty
 * @param mode
 */
void forward_set_duty(float selected_duty, int8_t mode) {

	if (mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A,
			selected_duty)!=ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}

#if	DEBUG_PRINT_BUTTON
		printf("duty:%f equal %0.1f us MODE %d\n", selected_duty, (float) (PWM_COEFF * selected_duty), mode);
#endif
}
