/*
 * can_driver.c
 *
 *  Created on: 15.10.2020
 *      Author: cleme
 */

#include "can_driver.h"



static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_100KBITS();

//Filter all other IDs except MSG_ID
//static const can_filter_config_t f_config = { .acceptance_code = (MSG_ID << 21),
//		.acceptance_mask = ~(CAN_STD_ID_MASK << 21), .single_filter = true };

static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

//Set to NO_ACK mode due to self testing with single module
static const twai_general_config_t g_config =
		TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO_NUM, CAN_RX_GPIO_NUM, TWAI_MODE_NO_ACK);



/* --------------------------- Tasks and Functions -------------------------- */

void can_tx_task(void *arg) {

	uint8_t cmd = 0x03;
	uint32_t id = 0x0;
	twai_message_t tx_msg = {.data_length_code = 4,
			.identifier = (id |
					((uint32_t)cmd << 8)),
			.extd = 1, .ss=0, .self=1};

	while (1) {

		//int32_t duty = 80 * 10000;
		int32_t rpm = 25500;
		tx_msg.data[0] = rpm >> 24;
		tx_msg.data[1] = rpm >> 16;
		tx_msg.data[2] = rpm >> 8;
		tx_msg.data[3] = rpm;

//
//		esp_err_t rtn = twai_transmit(&tx_msg, portMAX_DELAY);
//		twai_status_info_t status_info;
//		twai_get_status_info(&status_info);
//		ESP_LOGI(pcTaskGetTaskName(0), "status_info.state=%d",status_info.state);
//
//		if (rtn == ESP_OK) {
//		    printf("Message queued for transmission\n");
//		    ESP_LOGI(pcTaskGetTaskName(0),"twai_receive identifier=0x%x flags=0x%x-%x-%x data_length_code=%d",
//		    		tx_msg.identifier, tx_msg.flags, tx_msg.extd, tx_msg.rtr, tx_msg.data_length_code);
//		} else {
//			twai_clear_transmit_queue();
//			printf("Failed to queue message for transmission\n");
//		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void can_rx_task(void *arg) {
	twai_message_t rx_message;
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
	/*
	esp_err_t rtn = twai_receive(&rx_msg, 0);
	                if (rtn == ESP_OK) {
	                        ESP_LOGI(pcTaskGetTaskName(0),"twai_receive identifier=0x%x flags=0x%x-%x-%x data_length_code=%d",
	                                rx_msg.identifier, rx_msg.flags, rx_msg.extd, rx_msg.rtr, rx_msg.data_length_code);
	*/
//    }
	while (1) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

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
    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
    	return false;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
        ESP_LOGI(EXAMPLE_TAG, "CAN running");
    } else {
        printf("Failed to start driver\n");
    	return false;
    }


	return true;
}
