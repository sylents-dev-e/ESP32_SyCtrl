/*
 * wifi_driver.h
 *
 *  Created on: 30.04.2020
 *      Author: cleme
 */

#ifndef MAIN_WIFI_DRIVER_H_
#define MAIN_WIFI_DRIVER_H_

#include "main.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "http_webserver.h"
#include "esp_wifi.h"

#define WIFI_AP_SSID 				CONFIG_ESP_WIFI_SSID
#define WIFI_AP_PASSWORD 			CONFIG_ESP_WIFI_PASSWORD
#define WIFI_MAX_STA 				CONFIG_ESP_WIFI_MAXSTA

#define SOFT_AP_IP_ADDRESS_1 		192
#define SOFT_AP_IP_ADDRESS_2 		168
#define SOFT_AP_IP_ADDRESS_3 		5
#define SOFT_AP_IP_ADDRESS_4 		18

#define SOFT_AP_GW_ADDRESS_1 		192
#define SOFT_AP_GW_ADDRESS_2 		168
#define SOFT_AP_GW_ADDRESS_3 		5
#define SOFT_AP_GW_ADDRESS_4 		20

#define SOFT_AP_NM_ADDRESS_1 		255
#define SOFT_AP_NM_ADDRESS_2 		255
#define SOFT_AP_NM_ADDRESS_3 		255
#define SOFT_AP_NM_ADDRESS_4 		0

#ifdef __cplusplus
extern "C" {
#endif
int init_wifi(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_WIFI_DRIVER_H_ */
