/*
 * adc_driver.c
 *
 *  Created on: 25.03.2020
 *      Author: cleme
 */

#include "adc_driver.h"

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;     //GPIO36 if ADC1,
/*
 * - 0dB attenuaton (ADC_ATTEN_DB_0) between 100 and 950mV
 * - 2.5dB attenuation (ADC_ATTEN_DB_2_5) between 100 and 1250mV
 * - 6dB attenuation (ADC_ATTEN_DB_6) between 150 to 1750mV
 * - 11dB attenuation (ADC_ATTEN_DB_11) between 150 to 2450mV
 */
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

static void check_efuse() {
	//Check TP is burned into eFuse
	if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
		printf("eFuse Two Point: Supported\n");
	} else {
		printf("eFuse Two Point: NOT supported\n");
	}

	//Check Vref is burned into eFuse
	if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
		printf("eFuse Vref: Supported\n");
	} else {
		printf("eFuse Vref: NOT supported\n");
	}
}

static void print_char_val_type(esp_adc_cal_value_t val_type) {
	if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
		printf("Characterized using Two Point Value\n");
	} else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
		printf("Characterized using eFuse Vref\n");
	} else {
		printf("Characterized using Default Vref\n");
	}
}

void adc_input(void *arg) {

	//Check if Two Point or Vref are burned into eFuse
	check_efuse();

	//Check WDT
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	//Configure ADC
	if (unit == ADC_UNIT_1) {
		adc1_config_width(ADC_WIDTH_BIT_12);
		adc1_config_channel_atten(channel, atten);
	} else {
		adc2_config_channel_atten((adc2_channel_t) channel, atten);
	}

	//Characterize ADC
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten,
			ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
	print_char_val_type(val_type);

	//Continuously sample ADC1
	while (1) {

		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
		uint32_t adc_reading = 0;

		//Multisampling
		for (int i = 0; i < NO_OF_SAMPLES; i++) {
			if (unit == ADC_UNIT_1) {
				adc_reading += adc1_get_raw((adc1_channel_t) channel);
			} else {
				int raw;
				adc2_get_raw((adc2_channel_t) channel, ADC_WIDTH_BIT_12, &raw);
				adc_reading += raw;
			}
		}

		// Multisampling average
		adc_reading /= NO_OF_SAMPLES;



		// Send data to pwm
		xQueueSend(xQueue, &adc_reading, (TickType_t ) 10);

		// Send data to display
		xQueueSend(queue_to_display_adc, &adc_reading, (TickType_t ) 10);

#if DEBUG_PRINT
		// Convert adc_reading to voltage in mV
		uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
		printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
#endif

	}
	vTaskDelete(NULL);
}

