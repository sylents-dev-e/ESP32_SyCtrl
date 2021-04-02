/*
 * display_driver.c
 *
 *  Created on: 09.04.2020
 *      Author: cleme
 */

#include "display_driver.h"
#include "font8x8_basic.h"

#define tag "SSD1306"

static TaskHandle_t task_handles[portNUM_PROCESSORS];

void ssd1306_init() {
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
			true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_MUX_RATIO, true); // set MUX ratio
	i2c_master_write_byte(cmd, 0x1F, true); 		  		// 0-31

	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_OFFSET, true); // set display offset
	i2c_master_write_byte(cmd, 0x0, true);						// 0 offset

	i2c_master_write_byte(cmd, 0x40, true); // set display start line

	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping

	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // Set COM Output Scan Direction

	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_PIN_MAP, true);	// Set COM Pins hardware
	i2c_master_write_byte(cmd, 0x02, true);

	i2c_master_write_byte(cmd, 0xA4, true); //  Resume to RAM content display

	i2c_master_write_byte(cmd, 0xA6, true); // display normal mode

	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true); //Set Osc Frequency
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true); // enable charge pump
	i2c_master_write_byte(cmd, 0x14, true);

	i2c_master_write_byte(cmd, 0xAF, true);

	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(tag, "OLED configured successfully");
	} else {
		ESP_LOGE(tag, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);

	uint8_t zero[128] = { 0 };
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
				true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}

void task_ssd1306_display_pattern(void *ignore) {
	i2c_cmd_handle_t cmd;
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	for (uint8_t i = 0; i < 8; i++) {
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
				true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		for (uint8_t j = 0; j < 128; j++) {
			i2c_master_write_byte(cmd, 0xFF >> (j % 8), true);
		}
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

	vTaskDelete(NULL);
}

void task_ssd1306_display_clear(void *ignore) {
	i2c_cmd_handle_t cmd;
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	uint8_t zero[128] = { 0 };

	//while (1){
	CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
	for (uint8_t i = 0; i < 8; i++) {
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
				true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
	//}
	//vTaskDelete(NULL);
}

void task_ssd1306_contrast(void *ignore) {
	i2c_cmd_handle_t cmd;
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	uint8_t contrast = 0;
	uint8_t direction = 1;
	while (true) {
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
				true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
		i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
		i2c_master_write_byte(cmd, contrast, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		vTaskDelay(1 / portTICK_PERIOD_MS);

		contrast += direction;
		if (contrast == 0xFF) {
			direction = -1;
		}
		if (contrast == 0x0) {
			direction = 1;
		}
	}
	vTaskDelete(NULL);
}

void task_ssd1306_scroll(void *ignore) {
	esp_err_t espRc;
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
			true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, 0x29, true); // vertical and horizontal scroll (p29)
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x07, true);
	i2c_master_write_byte(cmd, 0x01, true);
	i2c_master_write_byte(cmd, 0x3F, true);

	i2c_master_write_byte(cmd, 0xA3, true); // set vertical scroll area (p30)
	i2c_master_write_byte(cmd, 0x20, true);
	i2c_master_write_byte(cmd, 0x40, true);

	i2c_master_write_byte(cmd, 0x2F, true); // activate scroll (p29)

	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(tag, "Scroll command succeeded");
	} else {
		ESP_LOGE(tag, "Scroll command failed. code: 0x%.2X", espRc);
	}

	i2c_cmd_link_delete(cmd);

	while (1)
		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);

	vTaskDelete(NULL);
}

void task_ssd1306_display_text(void *arg_text) {
	char *text = NULL;
	char *text2 = "";
	uint32_t mode = 0;
	uint32_t steering_mvolt = 0;
	float steering_duty = 0.0;

	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	while (1) {
		//task_ssd1306_display_clear(NULL);
		//vTaskDelay(10/portTICK_PERIOD_MS);
		i2c_cmd_handle_t cmd;

		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
		xQueueReceive(queue_to_display_dig, &mode, (TickType_t ) 10);
		xQueueReceive(queue_to_display_adc, &steering_mvolt, (TickType_t ) 10);

		if (mode == 0) {
			text = "m1 duty: 850us";
		}

		else if (mode == 1) {
			text = "m2 duty: 920us";
		}

		else if (mode == 2) {
			text = "m3 duty: 1400us";
		}

		else if (mode == 3) {
			text = "m4 duty: 1800us";
		}

		else if (mode == 4) {
			text = "m5 duty: 2200us";
		} else {
			text = "error or -1";
		}

		uint8_t text_len = strlen(text);

		// start display the mode (throttle)
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
				true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
		i2c_master_write_byte(cmd, 0x00, true); // reset column
		i2c_master_write_byte(cmd, 0x10, true);
		i2c_master_write_byte(cmd, 0xB0, true); // page1

		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		for (uint8_t i = 0; i < text_len; i++) {

			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd,
					(OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t) text[i]], 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}

		// start re-calculating the steering data and display it
		// NOT IMPLEMENTED YET
//		steering_duty = ((float)steering_mvolt * STEERING_CALC) + 5.0;
//
//		if (steering_duty>=5.5){
//
//// Takes a long calculation time
//			int len = snprintf(NULL, 0, "%s%.1f%s%d%s","Right: ", steering_duty, "   ", steering_mvolt, " mV");
//			text2 = (char *)malloc(len + 1);
//			snprintf(text2, len + 1, "%s%.1f%s%d%s","Right: ", steering_duty, "   ", steering_mvolt, " mV");
//		}
//
//		cmd = i2c_cmd_link_create();
//		i2c_master_start(cmd);
//		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
//
//		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
//		i2c_master_write_byte(cmd, 0x00, true); // reset column
//		i2c_master_write_byte(cmd, 0x10, true);
//		i2c_master_write_byte(cmd, 0xB1, true); // page1
//
//		i2c_master_stop(cmd);
//		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
//		i2c_cmd_link_delete(cmd);
//
//		uint8_t text_len2 = strlen(text2);
//
//		for (uint8_t i = 0; i < text_len2; i++) {
//
//			cmd = i2c_cmd_link_create();
//			i2c_master_start(cmd);
//			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
//
//			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
//			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text2[i]], 8, true);
//
//			i2c_master_stop(cmd);
//			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
//			i2c_cmd_link_delete(cmd);
//		}
//
//
//		free(text2);
		//free(text);
	}

	vTaskDelete(NULL);
}

bool display_start(void) {
	ssd1306_init();
	//task_ssd1306_display_clear(NULL);
	//xTaskCreate(&task_ssd1306_display_pattern, "ssd1306_display_pattern",  2048, NULL, 6, NULL);
//	xTaskCreate(&task_ssd1306_display_clear, "ssd1306_display_clear",  2048, NULL, 6, NULL);
	vTaskDelay(100 / portTICK_PERIOD_MS);

	//xTaskCreate(&task_ssd1306_contrast, "ssid1306_contrast", 2048, NULL, 6, NULL);
	//xTaskCreatePinnedToCore(task_ssd1306_scroll, "ssid1306_scroll", 2048, NULL, (tskIDLE_PRIORITY + 3), &task_handles[1],1);

	//xTaskCreatePinnedToCore(task_ssd1306_display_clear, "ssd1306_display_clear", 2048, NULL, (tskIDLE_PRIORITY + 3), &task_handles[1],1);
	//vTaskDelay(10/portTICK_PERIOD_MS);
	//xTaskCreatePinnedToCore(task_ssd1306_scroll, "ssid1306_scroll", 2048, NULL, (tskIDLE_PRIORITY + 3), &task_handles[1],1);
	xTaskCreatePinnedToCore(task_ssd1306_display_text, "ssd1306_display_text",
			2048, NULL, (tskIDLE_PRIORITY + 3), &task_handles[1], 1);
	return true;
}
