/*
 * spi_driver.c
 *
 *  Created on: 24.04.2020
 *      Author: cleme
 */

#include "spi_driver.h"

/**
 * @fn spi_device_handle_t init_spi(int8_t, uint8_t, uint8_t, uint32_t, uint32_t, uint8_t, uint8_t, uint8_t, void*, uint8_t, uint8_t)
 * @brief spi device and bus configuration
 *
 * @param miso
 * @param mosi
 * @param sclk
 * @param max_transfer
 * @param clock
 * @param mode
 * @param spi_cs
 * @param queue_size
 * @param host
 * @param dma_channel
 * @return spi structure
 */
spi_device_handle_t init_spi(int8_t miso, uint8_t mosi, uint8_t sclk,
		uint32_t max_transfer, uint32_t clock, uint8_t mode, uint8_t spi_cs,
		uint8_t queue_size, void (*ptr), uint8_t host, uint8_t dma_channel) {

	esp_err_t ret;
	spi_device_handle_t spi;
	spi_bus_config_t buscfg = { .miso_io_num = PIN_NUM_MISO, .mosi_io_num =
			PIN_NUM_MOSI, .sclk_io_num = PIN_NUM_CLK, .quadwp_io_num = -1,
			.quadhd_io_num = -1, .max_transfer_sz = max_transfer };

	spi_device_interface_config_t devcfg = { .clock_speed_hz = clock, .mode =
			mode, .spics_io_num = spi_cs, .queue_size = queue_size, .pre_cb =
			ptr, .flags = SPI_DEVICE_HALFDUPLEX };
	//Initialize the SPI bus
	ret = spi_bus_initialize(host, &buscfg, dma_channel);
	ESP_ERROR_CHECK(ret);
	ret = spi_bus_add_device(host, &devcfg, &spi);
	ESP_ERROR_CHECK(ret);

	return spi;
}

