/*
 * main.h
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
 * @brief reading from push buttons and setting pwm-output to throttle and steering
 */

void app_main(void) {
	// initialization
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

	// create inter task communication
	configASSERT(queue_semaphore_create());

	// init drivers
	configASSERT(driver_cfg());

	// test display start SSD1306 NOT IN USE
	// display_start();

	xTaskCreatePinnedToCore(adc_input, "adc_input", 2048, NULL,
			(tskIDLE_PRIORITY + 1), &task_handles[0], 0);

	while (1) {
		/* WTD timing config to avoid the following error:
		 * "Task watchdog got triggered. The following tasks did not reset the watchdog in time: - IDLE (CPU 0)
		 */
		TIMERG0.wdt_wprotect = 0x50D83AA1;
		TIMERG0.wdt_feed = 1;
		TIMERG0.wdt_wprotect = 0;

// just to check if scheduler is running
//		printf("scheduler %d\n", xTaskGetSchedulerState());
//		vTaskDelay(pdMS_TO_TICKS(1000));

	}
}

bool driver_cfg(void) {


	// init i2c driver
	i2c_master_init();

	// init pwm unit
	configASSERT(pwm_generation());

	//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	// init buttons GPIO
	configASSERT(button_config(LEFT));
	configASSERT(button_config(RIGHT));
	configASSERT(button_config(UP));
	configASSERT(button_config(DOWN));

	spi_device_handle_t spi;

	// init spi driver
#ifdef CONFIG_LCD_OVERCLOCK
    //Clock out at 26 MHz
	spi = init_spi(PIN_NUM_MISO, PIN_NUM_MOSI, PIN_NUM_CLK, PARALLEL_LINES*320*2+8, 26*1000*1000,
	    		0, PIN_NUM_CS, 7, &lcd_spi_pre_transfer_callback, LCD_HOST, DMA_CHAN);
	lcd_init(spi);
#else
    //Clock out at 10 MHz
	spi = init_spi(PIN_NUM_MISO, PIN_NUM_MOSI, PIN_NUM_CLK, PARALLEL_LINES*320*2+8, 10*1000*1000,
    		0, PIN_NUM_CS, 7, &lcd_spi_pre_transfer_callback, LCD_HOST, DMA_CHAN);
//cd_init(spi);
	lcd_start(spi);
	//lcd_fill(0xF800,spi);
	//lcd_draw_string(0,0, "hello", 0xFFFF,0x07E0, spi);
#endif


	//disp_driver_init(true);
	//nd_lines(spi,8,(uint16_t*)0x18);
	//LED
//	gpio_pad_select_gpio(2);
//	gpio_set_direction(2, GPIO_MODE_OUTPUT);
//	gpio_set_level(2, 0);

	return true;
}

bool queue_semaphore_create(void) {
	//message queue for digital button com
	queue_button = xQueueCreate(1, sizeof(uint32_t));
	// check if Queue was successfully created
	configASSERT(queue_button);

	//message queue for adc joystick
	xQueue = xQueueCreate(1, sizeof(uint32_t));
	// check if Queue was successfully created
	configASSERT(xQueue);

	//message queue for button to display
	queue_to_display_dig = xQueueCreate(1, sizeof(uint32_t));
	// check if Queue was successfully created
	configASSERT(queue_to_display_dig);

	//message queue for adc joystick to display
	queue_to_display_adc = xQueueCreate(1, sizeof(uint32_t));
	// check if Queue was successfully created
	configASSERT(queue_to_display_adc);

	// semaphore for buttons
	vSemaphoreCreateBinary(CountSemaphore);
	// check if Queue was successfully created
	configASSERT(CountSemaphore);



	return true;
}
