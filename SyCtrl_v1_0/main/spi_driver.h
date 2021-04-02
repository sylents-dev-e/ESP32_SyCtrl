/*
 * spi_master.h
 *
 *  Created on: 24.04.2020
 *      Author: cleme
 */

#ifndef MAIN_SPI_DRIVER_H_
#define MAIN_SPI_DRIVER_H_

#include "main.h"
#include "spi_lcd_ILI9341.h"

#ifdef __cplusplus
extern "C" {
#endif
spi_device_handle_t init_spi (uint8_t miso, uint8_t mosi, uint8_t sclk, uint32_t max_transfer,
				uint32_t clock, uint8_t mode, uint8_t spi_cs, uint8_t queue_size,
				void (*ptr), uint8_t host, uint8_t dma_channel);

bool disp_spi_is_busy(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_SPI_DRIVER_H_ */
