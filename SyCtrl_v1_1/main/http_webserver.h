/*
 * http_webserver.h
 *
 *  Created on: 04.05.2020
 *      Author: cleme
 */
#ifndef HTTP_WEBSERVER_H_
#define HTTP_WEBSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"
#include "wifi_driver.h"
#include <esp_http_server.h>

// Port for custom webserver
#define SERVER_PORT 3500

// Keys for JSON incomming data
#define JSON_OP_MOVE_KEY "move"
#define JSON_THROTTLE_KEY "throttle_gear"
#define JSON_STEERING_KEY "steering"
#define JSON_COMMAND_KEY "command"
#define JSON_SEND_KEY "send_data"
#define JSON_SEND_DIR "send_dir"

void stopHttpServer(void);
void server_start(void);
void json_operations(void);
#ifdef __cplusplus
}
#endif

#endif
