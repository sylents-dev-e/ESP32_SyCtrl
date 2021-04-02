/*
 * data_logging.c
 *
 *  Created on: 11.02.2021
 *      Author: cleme
 */

#include <data_logging.h>

void data_logging_task(void *arg) {

#if DEBUG_TASK_STACK
	UBaseType_t uxHighWaterMark;
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
#endif

	//Check WDT
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

	while (1) {

		CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);

		vTaskDelay(50);

#if DEBUG_TASK_STACK
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		printf("%d KB %s\n", uxHighWaterMark, "data_logging_task");
		vTaskDelay(500);
#endif

	}
	vTaskDelete(NULL);
}
