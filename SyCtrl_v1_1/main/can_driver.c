/*
 * can_driver.c
 *
 *  Created on: 15.10.2020
 *      Author: cleme
 */

#include "can_driver.h"

static const can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();

//Filter all other IDs except MSG_ID
//static const can_filter_config_t f_config = { .acceptance_code = (MSG_ID << 21),
//		.acceptance_mask = ~(CAN_STD_ID_MASK << 21), .single_filter = true };

static const can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

//Set to NO_ACK mode due to self testing with single module
static const can_general_config_t g_config =
		CAN_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, CAN_MODE_NO_ACK);

/* --------------------------- Tasks and Functions -------------------------- */

void can_tx_task(void *arg) {
//    can_message_t tx_msg = {.data_length_code = 1, .identifier = MSG_ID, .self = 1};
//    for (int iter = 0; iter < 5; iter++) {
//        xSemaphoreTake(can_tx_sem, portMAX_DELAY);
//        for (int i = 0; i < NO_OF_MSGS; i++) {
//            //Transmit messages using self reception request
//            tx_msg.data[0] = i;
//            ESP_ERROR_CHECK(can_transmit(&tx_msg, portMAX_DELAY));
//            vTaskDelay(pdMS_TO_TICKS(10));
//        }
//    }
//    vTaskDelete(NULL);
	while (1) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void can_rx_task(void *arg) {
//    can_message_t rx_message;
//
//    for (int iter = 0; iter < 5; iter++) {
//        xSemaphoreTake(can_rx_sem, portMAX_DELAY);
//        for (int i = 0; i < NO_OF_MSGS; i++) {
//            //Receive message and print message data
//            ESP_ERROR_CHECK(can_receive(&rx_message, portMAX_DELAY));
//            ESP_LOGI(EXAMPLE_TAG, "Msg received - Data = %d", rx_message.data[0]);
//        }
//        //Indicate to control task all messages received for this iteration
//        xSemaphoreGive(can_ctrl_sem);
//    }
	while (1) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	ESP_ERROR_CHECK(can_stop());
	ESP_ERROR_CHECK(can_driver_uninstall());
	vTaskDelete(NULL);
}

void can_ctrl_task(void *arg) {
	while (1) {
		vTaskDelay(100 / portTICK_PERIOD_MS);        //Delay then start next iteration
	}
	//xSemaphoreGive(can_done_sem);
	vTaskDelete(NULL);
}

bool can_initialization(void) {
	//Install CAN driver
	ESP_ERROR_CHECK(can_driver_install(&g_config, &t_config, &f_config));
	ESP_ERROR_CHECK(can_start());
	ESP_ERROR_CHECK(can_reconfigure_alerts(CAN_ALERT_TX_IDLE,NULL));

	ESP_LOGI(EXAMPLE_TAG, "CAN running");

	can_message_t message;



	//Start control task
	//xSemaphoreGive(can_ctrl_sem);
	//Wait for all iterations and tasks to complete running
	//xSemaphoreTake(can_done_sem, portMAX_DELAY);

	//Cleanup
//    vSemaphoreDelete(tx_sem);
//    vSemaphoreDelete(rx_sem);
//    vSemaphoreDelete(ctrl_sem);
	//vQueueDelete(done_sem);
	return true;
}
