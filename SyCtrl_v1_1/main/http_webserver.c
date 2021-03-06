/*
 * http_webserver.c
 *
 *  Created on: 04.05.2020
 *      Author: cleme
 */

#include "http_webserver.h"

// Globals for JSON/HTTP server
static cJSON *json_root = NULL;
static cJSON *json_operation = NULL;
static cJSON *json_gear = NULL;
static cJSON *json_steering = NULL;
static cJSON *json_command = NULL;
static httpd_handle_t httpServerInstance = NULL;

static const char *TAG = "example";

/**
 * @fn esp_err_t post_handler(httpd_req_t*)
 * @brief uri POST handler function. receives and sends data from mobile App
 * 		  client to process it on MCUs pwm unit. Other status messages can
 * 		  be sent to obtain the system's state.
 * 		  In case the smartphone app is connected and enables its steering unit,
 * 		  the ADC won't be able to send data to pwm as its power will be turned
 * 		  to an off state and the Mutex is blocked by the app-steering.
 *
 *
 * @param req
 * @return
 */
esp_err_t post_handler(httpd_req_t *req) {

	char content[CONFIG_ESP_HTTP_CONTENT_BUFFER];

	static int32_t steering_bit_val = INIT_APP_STEERING_POS;
	int32_t steering_data = 0;
	int32_t steering_en_temp = 0;
	uint32_t steering_in_percent = 0;
	int32_t gear_from_pwm = 0;
	uint32_t steering_from_pwm = 0;
	static int32_t steering_en = 0; // default 0 so that adc is higher prior. for setting the steering
	int32_t gear_data = 0; // default gear 0

	size_t recv_size = MIN(req->content_len, sizeof(content));

	// Receive data
	int ret = httpd_req_recv(req, content, recv_size);
	if (ret <= 0) { /* 0 return value indicates connection closed */
		/* Check if timeout occurred */
		if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
			httpd_resp_send_408(req);
		}
		return ESP_FAIL;
	}

	/*****************************JSON PARSING START*****************************************/

	// see @ http_webserver.h for JSON Key value
	json_root = cJSON_Parse(content);
	printf("%s\n", cJSON_Print(json_root));

	json_operation = cJSON_GetObjectItemCaseSensitive(json_root,
	JSON_OP_MOVE_KEY);
	json_gear = cJSON_GetObjectItemCaseSensitive(json_operation,
	JSON_THROTTLE_KEY);
	json_steering = cJSON_GetObjectItemCaseSensitive(json_operation,
	JSON_STEERING_KEY);
	json_command = cJSON_GetObjectItemCaseSensitive(json_operation,
	JSON_COMMAND_KEY);

	// STEERING ENABLE (0 | 1) OR EMERGENCY (-1)
	if (cJSON_IsNumber(
			cJSON_GetObjectItemCaseSensitive(json_command, JSON_SEND_KEY))) {
		steering_en_temp = (cJSON_GetObjectItemCaseSensitive(json_command,
		JSON_SEND_KEY))->valueint;
		steering_en = steering_en_temp;
	}

	// GEAR
	if (steering_en_temp == -1) {
		gear_data = EMERGENCY;
	} else {
		if (cJSON_IsNumber(
				cJSON_GetObjectItemCaseSensitive(json_gear, JSON_SEND_KEY))) {
			gear_data = (cJSON_GetObjectItemCaseSensitive(json_gear,
					JSON_SEND_KEY))->valueint;
		}
	}

	// STEERING
	if (cJSON_IsNumber(
			cJSON_GetObjectItemCaseSensitive(json_steering, JSON_SEND_KEY))) {
		steering_data = (cJSON_GetObjectItemCaseSensitive(json_steering,
		JSON_SEND_KEY))->valueint;
	}

	/*****************************JSON PARSING END***********************************************/

	/*****************************TO PWM-THROTTLE UNIT START*****************************************/
	if ((gear_data == DOWN) || (gear_data == UP) || (gear_data == EMERGENCY)) {
		if ( xSemaphoreTake(ButtonThrottleAppSemaphore,
				(TickType_t ) 10) == pdTRUE) {
			xQueueOverwrite(queue_button, &gear_data);
			xSemaphoreGive(CountSemaphore);
			xSemaphoreGive(ButtonThrottleAppSemaphore);
			if (gear_data == EMERGENCY){
				xSemaphoreGive(ButtonSteeringAppSemaphore);
			}

		} else {
			__asm__ volatile ( "NOP" );
		}
	} else {
		__asm__ volatile ( "NOP" );
	}

	/*****************************TO PWM-THROTTLE UNIT END*****************************************/

	/***************************** TO PWM-STEERING UNIT START***************************************/
	if ((steering_en_temp != steering_en) || (steering_en == 1)) {
		xSemaphoreTake(ButtonSteeringAppSemaphore, (TickType_t ) 10);
		adc_off();

		// LEFT/RIGHT defined in main.h
		if ((steering_data == LEFT_FROM_APP)
				|| (steering_data == RIGHT_FROM_APP)) {

			if ((steering_data == LEFT_FROM_APP)) {
				steering_bit_val -= CONFIG_ESP_ADC_APP_EMULATION_STEPS;
				if (steering_bit_val < 1) {
					steering_bit_val = 0;
				}
			} else if (steering_data == RIGHT_FROM_APP) {
				steering_bit_val += CONFIG_ESP_ADC_APP_EMULATION_STEPS;
				if (steering_bit_val > (INIT_APP_STEERING_POS * 2) - 1) {
					steering_bit_val = (INIT_APP_STEERING_POS * 2) - 1;
				}
			}
		}
		xQueueOverwrite(xQueue, &steering_bit_val);

	} else {
		steering_bit_val = INIT_APP_STEERING_POS;
		adc_on();
		vTaskDelay(10);
		// no app-steering, semaphore is given to adc-unit instantly
		xSemaphoreGive(ButtonSteeringAppSemaphore);
	}

	/***************************** TO PWM-STEERING UNIT END*****************************************/

	// Receive gear mode from PWM unit and parse it back into a JSON string
	xQueueReceive(queue_to_json_to_app, &gear_from_pwm, (TickType_t ) 10);
	cJSON_AddNumberToObject(json_gear, JSON_SEND_KEY, gear_from_pwm);

	// Steering percentage calculation
	xQueueReceive(queue_to_json_to_app_adc, &steering_from_pwm,
			(TickType_t ) 10);

	if (steering_from_pwm < INIT_APP_STEERING_POS) {
		steering_in_percent = 100
				- (uint32_t) (((float) steering_from_pwm
						/ (float) INIT_APP_STEERING_POS) * 100);

	} else if ((steering_from_pwm >= INIT_APP_STEERING_POS)
			&& (steering_from_pwm < (INIT_APP_STEERING_POS * 2))) {
		steering_in_percent =
				(uint32_t) (((float) (steering_from_pwm - INIT_APP_STEERING_POS)
						/ (float) INIT_APP_STEERING_POS) * 100);
	}

	if (steering_in_percent > 92) {
			steering_in_percent = 100;
		}


	cJSON_AddNumberToObject(json_steering, JSON_SEND_KEY, steering_in_percent);

	// respond steering direction to app
	if (steering_from_pwm < (INIT_APP_STEERING_POS-20)){
		cJSON_AddNumberToObject(json_steering, JSON_SEND_DIR, 1);
	}
	else if (steering_from_pwm > (INIT_APP_STEERING_POS+20)){
		cJSON_AddNumberToObject(json_steering, JSON_SEND_DIR, 0);
	}

	// Http server handles JSON as normal String with response type set to JSON
	const char *string = cJSON_Print(json_root);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	if (httpd_resp_send(req, string,
			strlen(string))==ESP_ERR_HTTPD_INVALID_REQ) {
		printf("invalid arg");
	}
	return ESP_OK;
}
/**
 * Structure for uri POST
 */
static const httpd_uri_t post = { .uri = "/post", .method = HTTP_POST,
		.handler = post_handler, .user_ctx = NULL };

/**
 * @fn esp_err_t http_404_error_handler(httpd_req_t*, httpd_err_code_t)
 * @brief Error handler for 404_ERROR
 *
 * @param req
 * @param err
 * @return
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
	if (strcmp("/get", req->uri) == 0) {
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
				"/get URI is not available");
		/* Return ESP_OK to keep underlying socket open */
		return ESP_OK;
	} else if (strcmp("/post", req->uri) == 0) {
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
				"/post URI is not available");
		/* Return ESP_FAIL to close underlying socket */
		return ESP_FAIL;
	}
	/* For any other URI send 404 and close socket */
	httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
	return ESP_FAIL;
}

/**
 * @fn httpd_handle_t startHttpServer(void)
 * @brief Start http server with specific parameters
 *
 * @return
 */
static httpd_handle_t startHttpServer(void) {

	// Custom Http configuration
	httpd_config_t httpServerConfiguration = HTTPD_DEFAULT_CONFIG();
	int32_t ret_queue = 0;
	httpServerConfiguration.server_port = CONFIG_ESP_HTTP_SERVER_PORT;
	httpServerConfiguration.recv_wait_timeout =
	CONFIG_ESP_HTTP_SERVER_RECEIVE_TIMEOUT;
	httpServerConfiguration.task_priority = tskIDLE_PRIORITY + 6;
	httpServerConfiguration.core_id = 1;
	httpServerConfiguration.stack_size = TASK_STACK_SIZE * 4;
	httpServerConfiguration.send_wait_timeout =
	CONFIG_ESP_HTTP_SERVER_TRANSMIT_TIMEOUT;

	// add uri handlers/structures
	if (httpd_start(&httpServerInstance, &httpServerConfiguration) == ESP_OK) {
		httpd_register_uri_handler(httpServerInstance, &post);
		ret_queue = ESP_OK;

		// to display
		xQueueSend(queue_http, &ret_queue, (TickType_t ) 10);
		return httpServerInstance;
	}
	// to display
	ret_queue = ESP_FAIL;
	xQueueSend(queue_http, &ret_queue, (TickType_t ) 10);

	ESP_LOGI(TAG, "Error starting server!");
	return NULL;
}

/**
 * @fn void stopHttpServer(void)
 * @brief Stop running http server
 *
 */
void stopHttpServer(void) {
	int32_t ret_queue = 0;
	if (httpServerInstance != NULL) {
		ESP_ERROR_CHECK(httpd_stop(httpServerInstance));

		// to display
		ret_queue = ESP_FAIL;
		xQueueSend(queue_http, &ret_queue, (TickType_t ) 10);
	}
}

/**
 * @fn void server_start(void)
 * @brief http start helper funtion
 *
 */
void server_start(void) {
	startHttpServer();
}

/**
 * @fn void json_operations(void)
 * @brief Create to JSON root object
 *
 */
void json_operations(void) {
	json_root = cJSON_CreateObject();
}
