/*
 * spi_lcd_ILI9341.h
 *
 *  Created on: 24.04.2020
 *      Author: cleme
 */

#ifndef MAIN_SPI_LCD_ILI9341_H_
#define MAIN_SPI_LCD_ILI9341_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif
void lcd_fill(spi_device_handle_t spi, uint16_t colour);
void lcd_setaddress(spi_device_handle_t spi, uint16_t x1, uint16_t y1,
		uint16_t x2, uint16_t y2);
void lcd_cmd(spi_device_handle_t spi, uint8_t cmd);
void lcd_data_queue_queue(spi_device_handle_t spi, uint8_t data);
void lcd_flush(spi_device_handle_t spi, uint16_t colour, uint16_t x, uint16_t y);
void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
uint8_t lcd_set_cursor_y(uint16_t y, spi_device_handle_t spi);
uint8_t lcd_set_cursor(uint16_t x, uint16_t y, spi_device_handle_t spi);
uint8_t lcd_set_cursor_x(uint16_t x, spi_device_handle_t spi);
uint8_t lcd_draw_pixel(uint16_t color, spi_device_handle_t spi);

void lcd_draw_circle(uint16_t xm, uint16_t ym, uint16_t r, uint16_t color,spi_device_handle_t spi);
void lcd_draw_pixel_at(uint16_t x, uint16_t y, uint16_t color,spi_device_handle_t spi);

void lcd_draw_string(uint16_t x, uint16_t y, const char *pS, uint16_t fg_color,
		uint16_t bg_color, spi_device_handle_t spi);
void lcd_draw_char(uint16_t x, uint16_t y, uint16_t fIndex, uint16_t fg_color,
		uint16_t bg_color, spi_device_handle_t spi);
void lcd_start(spi_device_handle_t spi);

#ifdef __cplusplus
}
#endif

#define LCD_WIDTH   		320
#define LCD_HEIGHT  		240

#define CONFIG_LCD_OVERCLOCK 0

#define LCD_HOST    		HSPI_HOST
#define DMA_CHAN    		2

#define PIN_NUM_MISO 		25
#define PIN_NUM_MOSI 		23
#define PIN_NUM_CLK  		19
#define PIN_NUM_CS  		22

#define PIN_NUM_DC   		21
#define PIN_NUM_RST  		18
#define PIN_NUM_BCKL 		5

#define BLACK       		0x0000 /*   0,   0,   0 */
#define NAVY        		0x000F
#define DARKGREEN   		0x03E0
#define DARKCYAN    		0x03EF
#define MAROON     			0x7800
#define PURPLE     			0x780F
#define OLIVE       		0x7BE0
#define LIGHTGREY   		0xC618
#define DARKGREY    		0x7BEF
#define BLUE        		0x001F
#define GREEN       		0x07E0
#define CYAN        		0x07FF
#define RED         		0xF800
#define MAGENTA     		0xF81F
#define YELLOW      		0xFFE0
#define WHITE       		0xFFFF
#define ORANGE      		0xFD20
#define GREENYELLOW 		0xAFE5
#define PINK        		0xF81F

//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 		16

#endif /* MAIN_SPI_LCD_ILI9341_H_ */
