/*
 * data_ILI9341.c
 *
 *  Created on: 12.05.2020
 *      Author: cleme
 */

#include "data_ILI9341.h"

/**
 * @fn void lv_tick_task(void*)
 * @brief Simple tick task inside the instruction RAM
 *
 * @param arg
 */
static void IRAM_ATTR lv_tick_task(void *arg) {
	(void) arg;

	lv_tick_inc(portTICK_RATE_MS);
}

/**
 * @fn void guiTask(void*)
 * @brief Display task, usage of LittlevGL Library
 * 		  {@link}
 * @see <a href="https://docs.lvgl.io">https://docs.lvgl.io</a>
 * @param arg
 */
void guiTask(void *arg) {

#if DEBUG_TASK_STACK
	UBaseType_t uxHighWaterMark;
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
#endif

	// WTD init for that task
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	// strings for display output
	char *str = NULL;

	int32_t gear_from_pwm = 0;
	int32_t val = 0;
	// not 0 initialized because 0 would be ESP_OK
	int32_t status_http = 1;

	// constant added strings
	const char *str_gear = "Gear : ";

	// init lib + drivers
	lv_init();
	lvgl_driver_init();

	static lv_color_t buf1[DISP_BUF_SIZE];
	static lv_color_t buf2[DISP_BUF_SIZE];
	static lv_disp_buf_t disp_buf;
	lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;

	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

#if CONFIG_LVGL_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

	const esp_timer_create_args_t periodic_timer_args = { .callback =
			&lv_tick_task,
	/* name is optional, but may help identify the timer when debugging */
	.name = "periodic_gui" };
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	//On ESP32 it's better to create a periodic task instead of esp_register_freertos_tick_hook
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000)); //10ms (expressed as microseconds)

	lv_obj_t *scr = lv_disp_get_scr_act(NULL); /*Get the current screen*/

	// background settings
	static lv_style_t style_screen;
	lv_style_copy(&style_screen, &lv_style_plain);
	style_screen.body.main_color = LV_COLOR_LIME;
	//style_screen.body.grad_color = LV_COLOR_LIME;
	style_screen.body.opa = LV_OPA_100;
	lv_obj_set_style(lv_scr_act(), &style_screen);


	// create label for gear string
	lv_obj_t *label1 = lv_label_create(scr, NULL);
	lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

	// create label for http string
	lv_obj_t *label3 = lv_label_create(scr, NULL);
	lv_obj_align(label3, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 40);

	// create label for version string
	lv_obj_t *label2 = lv_label_create(scr, NULL);
	lv_obj_align(label2, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
	lv_label_set_recolor(label2, true);
	lv_label_set_static_text(label2, SW_VERSION);

/********************************SLIDER BEGINN********************************************/
	static lv_style_t style_bg;
	static lv_style_t style_indic;
	static lv_style_t style_knob;

	lv_style_copy(&style_bg, &lv_style_pretty);
	style_bg.body.main_color = LV_COLOR_AQUA;
	style_bg.body.grad_color = LV_COLOR_AQUA;
	style_bg.body.radius = LV_RADIUS_CIRCLE;
	style_bg.body.opa = LV_OPA_50;
	style_bg.body.border.color = LV_COLOR_WHITE;

	lv_style_copy(&style_indic, &lv_style_pretty_color);
	style_indic.body.main_color = LV_COLOR_AQUA;
	style_indic.body.grad_color = LV_COLOR_AQUA;
	style_indic.body.opa = LV_OPA_0;
	style_indic.body.radius = LV_RADIUS_CIRCLE;
	style_indic.body.shadow.width = 0;
	style_indic.body.shadow.color = LV_COLOR_AQUA;
	style_indic.body.padding.left = 0;
	style_indic.body.padding.right = 0;
	style_indic.body.padding.top = 0;
	style_indic.body.padding.bottom = 0;

	lv_style_copy(&style_knob, &lv_style_pretty);
	style_knob.body.radius = LV_RADIUS_CIRCLE;
	style_knob.body.main_color = LV_COLOR_NAVY;
	style_knob.body.opa = LV_OPA_100;
	style_knob.body.padding.top = 10;
	style_knob.body.padding.bottom = 10;
	lv_obj_t *slider1 = lv_slider_create(lv_disp_get_scr_act(NULL), NULL);
	lv_slider_set_style(slider1, LV_SLIDER_STYLE_BG, &style_bg);
	lv_slider_set_style(slider1, LV_SLIDER_STYLE_INDIC, &style_indic);
	lv_slider_set_style(slider1, LV_SLIDER_STYLE_KNOB, &style_knob);

	lv_obj_set_size(slider1, 300, 40);
	lv_obj_align(slider1, NULL, LV_ALIGN_CENTER, 0, 0);

	// set to 12 bit resolution
	lv_slider_set_range(slider1, 0, 4096);
/********************************END SLIDER********************************************/

	while (1) {
		// give response to WDT
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);

		vTaskDelay(1 / portTICK_PERIOD_MS);


		// allocate memory for string passing to display
		str = malloc((320) * sizeof(char));

		xQueueReceive(queue_to_display_adc, &val, (TickType_t ) 10);

		lv_slider_set_value(slider1, val, LV_ANIM_ON);

		// receive gear from pwm_throttle
		xQueueReceive(queue_to_ili9341, &gear_from_pwm, (TickType_t ) 10);

		// receive http data
		xQueueReceive(queue_http, &status_http, (TickType_t ) 10);

		// make string from parts
		snprintf(str, sizeof(int32_t) + strlen(str_gear)+sizeof(char), "%s %d", str_gear,
				gear_from_pwm);

		// set text to the screen GEAR
		lv_label_set_text(label1, str);

		// set text to the screen HTTP_STATUS
		parse_http_status(status_http, label3);

		// free memory
		free(str);

		//Try to lock the semaphore, if success, call lvgl stuff
		if (xSemaphoreTake(GuiSemaphore, (TickType_t)10) == pdTRUE) {
			lv_task_handler();
			xSemaphoreGive(GuiSemaphore);
		}

#if DEBUG_TASK_STACK
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL);
		ets_printf("%d KB %s\n", uxHighWaterMark, "gui_task");
#endif

	}
	vTaskDelete(NULL);
}

/**
 * @fn void parse_http_status(int32_t, lv_obj_t*)
 * @brief display function to show the status of the http server
 * 		  that will be started when a device is connected to WiFi
 *
 * @param status
 * @param label3
 */
void parse_http_status(int32_t status, lv_obj_t *label3){

	const char *str_http = "HTTP server :";
	char *str2 = NULL;
	str2 = malloc((320) * sizeof(char));
	char *str_status = NULL;

	if (status == -1){
		str_status = "stopped";
		snprintf(str2, strlen(str_status)+strlen(str_http)+2*sizeof(char), "%s %s", str_http,
				str_status);
	}
	else if (status == 0){
		str_status = "started";
		snprintf(str2, strlen(str_status)+strlen(str_http)+2*sizeof(char), "%s %s", str_http,
				str_status);
	}
	else if (status == 1){
		str_status = "waiting";
		snprintf(str2, strlen(str_status)+strlen(str_http)+2*sizeof(char), "%s %s", str_http,
						str_status);
	}

	lv_label_set_text(label3, str2);
	free(str2);
}
