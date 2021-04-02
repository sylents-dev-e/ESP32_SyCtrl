/*
 * can_driver.h
 *
 *  Created on: 15.10.2020
 *      Author: cleme
 */

#ifndef MAIN_CAN_DRIVER_H_
#define MAIN_CAN_DRIVER_H_

#include "driver/gpio.h"
#include "driver/twai.h"


#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

bool can_initialization(void);
void can_ctrl_task(void *arg);
void can_tx_task(void *arg);
void can_rx_task(void *arg);

#ifdef __cplusplus
}
#endif

#define NO_OF_MSGS              3
#define CAN_TX_GPIO_NUM         26
#define CAN_RX_GPIO_NUM         27
#define MSG_ID                  0x555   //11 bit standard format ID
#define EXAMPLE_TAG             "CAN"


#endif /* MAIN_CAN_DRIVER_H_ */
