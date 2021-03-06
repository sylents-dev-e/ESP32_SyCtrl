/**
 * @file 	adc_driver.c
 *
 * @date 	Created on: 25.03.2020
 * @author 	Clemens Koernyefalvy
 */

#include "adc_driver.h"

static esp_adc_cal_characteristics_t *adc_chars;

// GPIO 36
static const adc_channel_t channel = ADC_CHANNEL_0;
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

#if DEBUG_TASK_STACK
	UBaseType_t uxHighWaterMark;
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
#endif

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

#if CONFIG_ESP_ADC_MULTISAMPLING_ON_OFF
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
		adc_reading /= CONFIG_ESP_NO_OF_SAMPLES;
#else
		adc_reading = adc1_get_raw((adc1_channel_t) channel);
#endif
		// Send data to pwm

		if ((LOW_LIMIT_STEERING < adc_reading) && (adc_reading < HIGH_LIMIT_STEERING)) {
			adc_reading = 2048;
		}
		if ((xSemaphoreTake(ButtonSteeringAppSemaphore, (TickType_t ) 10) == pdTRUE)) {
				xQueueOverwrite(xQueue, &adc_reading);
				xSemaphoreGive(ButtonSteeringAppSemaphore);
		}


#if DEBUG_PRINT_ADC
		// Convert adc_reading to voltage in mV

		printf("%d\n", adc_reading);
#endif

		vTaskDelay(50);

#if DEBUG_TASK_STACK
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		printf("%d KB %s\n", uxHighWaterMark, "adc_input");
		vTaskDelay(500);
#endif

	}
	vTaskDelete(NULL);
}

/**
 * @fn void adc_on(void)
 * @brief helper adc on function
 *
 */
void adc_on(void){
	adc_power_on();
}

/**
 * @fn void adc_off(void)
 * @brief helper adc off funtion
 *
 */
void adc_off(void){
	adc_power_off();
}
