/*
 * wifi_driver.c
 *
 *  Created on: 30.04.2020
 *      Author: cleme
 */

#include "wifi_driver.h"


static const char *TAG = "wifi softAP";


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
    	server_start();
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        stopHttpServer();
    }
}

//TODO SSID mit MAC verknüpfen
static void launchSoftAp(){
	esp_netif_t *ap_netif = NULL;
	esp_netif_init();
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ap_netif = esp_netif_create_default_wifi_ap();
	    ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));
	    esp_netif_ip_info_t ipAddressInfo;
	    memset(&ipAddressInfo, 0, sizeof(ipAddressInfo));
	    IP4_ADDR(
	        &ipAddressInfo.ip,
	        SOFT_AP_IP_ADDRESS_1,
	        SOFT_AP_IP_ADDRESS_2,
	        SOFT_AP_IP_ADDRESS_3,
	        SOFT_AP_IP_ADDRESS_4);
	    IP4_ADDR(
	        &ipAddressInfo.gw,
	        SOFT_AP_GW_ADDRESS_1,
	        SOFT_AP_GW_ADDRESS_2,
	        SOFT_AP_GW_ADDRESS_3,
	        SOFT_AP_GW_ADDRESS_4);
	    IP4_ADDR(
	        &ipAddressInfo.netmask,
	        SOFT_AP_NM_ADDRESS_1,
	        SOFT_AP_NM_ADDRESS_2,
	        SOFT_AP_NM_ADDRESS_3,
	        SOFT_AP_NM_ADDRESS_4);


	    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ipAddressInfo));
	    ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif));
//	    ESP_ERROR_CHECK(esp_event_loop_init(wifiEventHandler, NULL));
	    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
	                                                        ESP_EVENT_ANY_ID,
	                                                        &wifi_event_handler,
	                                                        NULL,
	                                                        NULL));
	    wifi_init_config_t wifiConfiguration = WIFI_INIT_CONFIG_DEFAULT();
	    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfiguration));
	    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	    wifi_config_t apConfiguration = {
	        .ap = {
	            .ssid = WIFI_AP_SSID,
	            .password = WIFI_AP_PASSWORD,
	            .ssid_len = strlen(WIFI_AP_SSID),
	            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
	            //.ssid_hidden = 0,
	            .max_connection = WIFI_MAX_STA,
	            .beacon_interval = 150,
	        },
	    };

	    esp_netif_get_ip_info(ap_netif, &ipAddressInfo);

	    // power save disabled
	    esp_wifi_set_ps(WIFI_PS_NONE);

	    printf("IP: %d.%d.%d.%d:%d", ipAddressInfo.ip.addr & 0xFF, (ipAddressInfo.ip.addr >> 8) & 0xFF, (ipAddressInfo.ip.addr >> 16) & 0xFF, (ipAddressInfo.ip.addr >> 24) & 0xFF, CONFIG_ESP_HTTP_SERVER_PORT);

	    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apConfiguration));
	    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	    ESP_ERROR_CHECK(esp_wifi_start());
}

int init_wifi(void)
{
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    launchSoftAp();

    return 0;
}


