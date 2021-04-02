/*
 * pwm.c
 *
 *  Created on: 24.03.2020
 *      Author: cleme
 *
 */

#include "pwm_driver.h"

static TaskHandle_t task_handles[portNUM_PROCESSORS];

/**
 * @fn bool pwm_gpio_init()
 * @brief initializing the PWM GPIO and setting default values
 * @return true on success
 */

static bool pwm_gpio_init() {
	printf("initializing mcpwm gpio...\n");

	/*
	 * setting default
	 */
#if MCPWM_GPIO_INIT
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0_THROTTLE_OUT);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0_STEERING_OUT);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_SYNC_0, GPIO_SYNC0_IN);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_SYNC_1, GPIO_SYNC1_IN);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_SYNC_2, GPIO_SYNC2_IN);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_FAULT_1, GPIO_FAULT1_IN);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_FAULT_2, GPIO_FAULT2_IN);
#else
    mcpwm_pin_config_t pin_config = {
        .mcpwm0a_out_num = GPIO_PWM0_THROTTLE_OUT,
		.mcpwm0b_out_num = GPIO_PWM0_STEERING_OUT,
        .mcpwm_sync0_in_num  = GPIO_SYNC0_IN,
        .mcpwm_sync1_in_num  = GPIO_SYNC1_IN,
        .mcpwm_sync2_in_num  = GPIO_SYNC2_IN,
        .mcpwm_fault1_in_num = GPIO_FAULT1_IN,
        .mcpwm_fault2_in_num = GPIO_FAULT2_IN,
    };
    mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);


#endif
	gpio_pulldown_en(GPIO_SYNC0_IN);   //Enable pull down on SYNC0  signal
	gpio_pulldown_en(GPIO_SYNC1_IN);   //Enable pull down on SYNC1  signal
	gpio_pulldown_en(GPIO_SYNC2_IN);   //Enable pull down on SYNC2  signal
	gpio_pulldown_en(GPIO_FAULT1_IN);  //Enable pull down on FAULT1 signal
	gpio_pulldown_en(GPIO_FAULT2_IN);  //Enable pull down on FAULT2 signal

	mcpwm_config_t init_pwm;
	init_pwm.frequency = 50;
	init_pwm.cmpr_b = MODE1;
	init_pwm.counter_mode = MCPWM_UP_COUNTER;
	init_pwm.duty_mode = MCPWM_DUTY_MODE_0; // active HIGH
	mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &init_pwm);

	return true;
}

/**
 * @brief Configure whole MCPWM module and create the tasks for steering and throttle
 * @return true on success
 */

bool pwm_generation(void) {
	//mcpwm gpio initialization
	configASSERT(pwm_gpio_init());

	//start motor
	configASSERT(init_motors());

	xTaskCreatePinnedToCore(pwm_throttle_task, "pwm_throttle_task",
	TASK_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2), &task_handles[0], 0);
	xTaskCreatePinnedToCore(pwm_steering_task, "pwm_steering_task",
	TASK_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2), &task_handles[1], 0);

	printf("Configuring Initial Parameters of mcpwm...\n");

	return true;
}

/**
 * @fn void pwm_throttle_task(void*)
 * @brief PWM creation through a duty cycle set by button ISR
 * @param arg not used
 */

void pwm_throttle_task(void *arg) {

	// WTD init for that task
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	// variables
	uint32_t c = 0;
	int8_t mode_cnt = 0;

	//DEBUG
	ets_printf("Debounce delay:%d ms\n", DEBOUNCE_DELAY);

	while (1) {

		// give response to WDT
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);

		// receiving sempahore and queue data from button ISR
		if ((xSemaphoreTake( CountSemaphore, portMAX_DELAY ) == pdTRUE)
				&& (xQueueReceive(queue_button, &c, portMAX_DELAY) == pdPASS)) {

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
				printf("count: %d\t not implemented\n", mode_cnt);
				break; //not yet implemented
			case 0:
				forward_set_duty(MODE1);
				break;
			case 1:
				forward_set_duty(PWM_START_CURRENT);
				break;
			case 2:
				forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT);
				break;
			case 3:
				forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT);
				break;
			case 4:
				forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT);
				break;
			default:
				forward_set_duty((PWM_INC * mode_cnt) + PWM_START_CURRENT);
				break;
			}

			xQueueSend(queue_to_display_dig, (void* ) &mode_cnt,
					(TickType_t ) 10);

			/* Wait a short time then clear any pending button pushes as a crude
			 method of debouncing the switch.  xSemaphoreTake() uses a block time of
			 zero this time so it returns immediately rather than waiting for the
			 interrupt to occur. */
			vTaskDelay(DEBOUNCE_DELAY); // set in button_driver.
			xSemaphoreTake(CountSemaphore, 0);
		}
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
	// WTD init for that task
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	// variables
	uint32_t adc_in = 0;

	while (1) {
		// give response to WDT
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);

		// receive message queue sent frrom ADC
		if ( xQueueReceive( xQueue, &adc_in,( TickType_t ) 10 ) == pdPASS) {
			if ((1870 < adc_in) && (adc_in < 1890)) {
				mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A,
				MODE_CENTER);
			} else {

				mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A,
						(((float) adc_in * STEERING_CALC) + 5.0));
			}
			// DEBUG
			//printf("adc_in: %d\n", adc_in);
		}
	}
	vTaskDelete(NULL);
}

/**
 * @fn bool init_motors(void)
 * @brief start motor with default values
 * 		  FAULT detection enabled
 * 		  SYNC mode enabled
 * @return true on success
 */
bool init_motors(void) {

	mcpwm_config_t pwm_config;
	pwm_config.frequency = 50;

	pwm_config.cmpr_b = MODE1;
	pwm_config.cmpr_a = MODE_CENTER;

	// MODES see @pwm_driver.h
	pwm_config.counter_mode = MCPWM_UP_COUNTER;
	pwm_config.duty_mode = MCPWM_DUTY_MODE_0; // active HIGH

	if (mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config) != ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	}

	/*
	 * NOT ENABLED
	 */
#if MCPWM_EN_DEADTIME
		//add rising edge delay or falling edge delay
		mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_RED_FED_FROM_PWMXA, 656, 67);  //Enable deadtime on PWM0A and PWM0B with red = (656)*100ns & fed = (67)*100ns on PWM0A and PWM0B generated from PWM0A
	#endif

	/*
	 * ENABLED
	 */
#if MCPWM_EN_FAULT
	mcpwm_fault_init(MCPWM_UNIT_0, MCPWM_HIGH_LEVEL_TGR, MCPWM_SELECT_F1); //Enable FAULT, when high level occurs on FAULT1 signal
	mcpwm_fault_init(MCPWM_UNIT_0, MCPWM_HIGH_LEVEL_TGR, MCPWM_SELECT_F2); //Enable FAULT, when high level occurs on FAULT2 signal
	mcpwm_fault_set_oneshot_mode(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_F2,
	MCPWM_FORCE_MCPWMXA_HIGH, MCPWM_FORCE_MCPWMXB_LOW); //Action taken on PWM0A and PWM0B, when FAULT2 occurs
	mcpwm_fault_set_oneshot_mode(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_F1,
	MCPWM_FORCE_MCPWMXA_LOW, MCPWM_FORCE_MCPWMXB_HIGH); //Action taken on PWM0A and PWM0B, when FAULT1 occurs
#endif

	/*
	 * ENABLED
	 */
#if MCPWM_EN_SYNC
	mcpwm_sync_enable(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_SELECT_SYNC0, 200); //Load counter value with 20% of period counter of mcpwm timer 1 when sync 0 occurs
#endif

	return true;
}

void forward_set_duty(float selected_duty) {

	if (mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B,
			selected_duty)!=ESP_OK) {
		ets_printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,
				__FUNCTION__);
		abort();
	} else {
		//DEBUG
		printf("duty:%f equal %0.1f us\n", selected_duty,
				(float) (PWM_COEFF * selected_duty));
	}
}
