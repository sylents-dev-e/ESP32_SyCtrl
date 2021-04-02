/*
 * spi_lcd_ILI9341.c
 *
 *  Created on: 24.04.2020
 *      Author: cleme
 */

#include "spi_lcd_ILI9341.h"
#include "font.h"

/**
 * @fn void lcd_cmd(spi_device_handle_t, const uint8_t)
 * @brief commands to spi_display, DC is set to 0
 *
 * @param spi
 * @param cmd
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd) {
	esp_err_t ret;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length = 8;                     //Command is 8 bits
	t.tx_buffer = &cmd;               //The data is the cmd itself
	t.user = (void*) 0;                //D/C needs to be set to 0
	ret = spi_device_queue_trans(spi, &t, portMAX_DELAY);
	//ret = spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
}

/**
 * @fn void lcd_data_queue(spi_device_handle_t, uint8_t)
 * @brief sending data to spi display via queue
 * 		  (alternative way via polling is commented)
 *
 * @param spi
 * @param data
 */
void lcd_data_queue(spi_device_handle_t spi, uint8_t data) {
	esp_err_t ret;
	static spi_transaction_t t;
	memset(&t, 0, sizeof(spi_transaction_t));
	t.user = (void*) 1;
	t.length = 8;
	t.tx_data[0] = data;
	t.flags = SPI_TRANS_USE_TXDATA;
	ret = spi_device_queue_trans(spi, &t, portMAX_DELAY);

//    static spi_transaction_t t;
//    memset(&t, 0, sizeof(t));
//    t.length=8;
//    t.flags=SPI_TRANS_USE_TXDATA;
//    t.tx_data[0]=data;
//    t.user=(void*)1;
//    ret=spi_device_polling_transmit(spi, &t);
	assert(ret==ESP_OK);
}

/**
 * @fn void lcd_spi_pre_transfer_callback(spi_transaction_t*)
 * @brief callback function for the DC value
 * 		  transmitting (void*) values are set by
 * 		  this function
 *
 * @param t
 */
void lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
	int dc = (int) t->user;
	gpio_set_level(PIN_NUM_DC, dc);
}

/**
 * @fn uint8_t lcd_set_cursor(uint16_t, uint16_t, spi_device_handle_t)
 * @brief set cursor on display to a specific postion
 *
 * @param x
 * @param y
 * @param spi
 * @return 0 on success
 */
uint8_t lcd_set_cursor(uint16_t x, uint16_t y, spi_device_handle_t spi) {
	if (lcd_set_cursor_x(x, spi) || lcd_set_cursor_y(y, spi)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * @fn uint8_t lcd_set_cursor_x(uint16_t, spi_device_handle_t)
 * @brief writing registers for x-postion
 *
 * @param x
 * @param spi
 * @return 0 on success
 */
uint8_t lcd_set_cursor_x(uint16_t x, spi_device_handle_t spi) {
	if (x >= LCD_WIDTH) {
		return EXIT_FAILURE;
	}
	lcd_cmd(spi, 0x2B);
	lcd_data_queue(spi, (x >> 8));
	lcd_data_queue(spi, (x & 0xFF));
	lcd_cmd(spi, 0x2c);

	return EXIT_SUCCESS;
}

/**
 * @fn uint8_t lcd_set_cursor_y(uint16_t, spi_device_handle_t)
 * @brief writing registers for y-postion
 *
 * @param y
 * @param spi
 * @return 0 on success
 */
uint8_t lcd_set_cursor_y(uint16_t y, spi_device_handle_t spi) {
	if (y >= LCD_HEIGHT) {
		return EXIT_FAILURE;
	}

	lcd_cmd(spi, 0x2A);
	lcd_data_queue(spi, (y >> 8));
	lcd_data_queue(spi, (y & 0xFF));
	lcd_cmd(spi, 0x2c);

	return EXIT_SUCCESS;
}

/**
 * @fn uint8_t lcd_draw_pixel(uint16_t, spi_device_handle_t)
 * @brief set pixel at given postion to colour value
 *
 * @param color
 * @param spi
 * @return 0 on success
 */
uint8_t lcd_draw_pixel(uint16_t color, spi_device_handle_t spi) {
	lcd_data_queue(spi, (color >> 8));
	lcd_data_queue(spi, (color & 0xFF));

	return EXIT_SUCCESS;
}

/**
 * @fn void lcd_draw_string(uint16_t, uint16_t, const char*, uint16_t, uint16_t, spi_device_handle_t)
 * @brief draws a string by invoking the lcd_draw_char function
 * 		  values are searched within the font.h array
 *
 * @param x
 * @param y
 * @param pS
 * @param fg_color
 * @param bg_color
 * @param spi
 */
void lcd_draw_string(uint16_t x, uint16_t y, const char *pS, uint16_t fg_color,
		uint16_t bg_color, spi_device_handle_t spi) {
	uint16_t lIndex, k;

	while (*pS) {

		// set index for font array
		lIndex = 0;
		for (k = 0; k < (*pS - ' '); k++) {
			lIndex += ((font[lIndex]) << 1) + 1;
		}

		// draw character
		lcd_draw_char(x, y, lIndex, fg_color, bg_color, spi);
		// move cursor
		x += font[lIndex];
		// next character
		pS++;
	}

//	for (int i = c; i < LCD_WIDTH; i++){
//		lcd_flush(spi,BLACK,x, i);
//	}
//	lcd_draw_char(x, y, 0, fg_color, bg_color, spi);
}

/**
 * @fn void lcd_draw_char(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, spi_device_handle_t)
 * @brief draw a single charater from the font.h character array
 *
 * @param x
 * @param y
 * @param fIndex
 * @param fg_color
 * @param bg_color
 * @param spi
 */
void lcd_draw_char(uint16_t x, uint16_t y, uint16_t fIndex, uint16_t fg_color,
		uint16_t bg_color, spi_device_handle_t spi) {
	uint8_t j, k, i;
	//lcd_flush(spi,BLACK,x,y);
	for (j = 0; j < font[fIndex]; j++) {
		ESP_ERROR_CHECK(lcd_set_cursor(x + font[fIndex] - j, y, spi));

		for (k = 0; k < FONT_HEIGHT; k++) {
			for (i = 0; i < 8; i++) {
				if (font[(fIndex + ((font[fIndex]) << 1)) - (j << 1) - k]
						& (0x80 >> i)) {
					ESP_ERROR_CHECK(lcd_draw_pixel(fg_color, spi));
				} else {
					ESP_ERROR_CHECK(lcd_draw_pixel(bg_color, spi));
				}
			}
		}
	}

}

/**
 * @fn void lcd_setaddress(spi_device_handle_t, uint16_t, uint16_t, uint16_t, uint16_t)
 * @brief adress setting for the screen fill option
 *
 * @param spi
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 */
void lcd_setaddress(spi_device_handle_t spi, uint16_t x1, uint16_t y1,
		uint16_t x2, uint16_t y2) {
	lcd_cmd(spi, 0x2A);
	lcd_data_queue(spi, (x1 >> 8));
	lcd_data_queue(spi, x1);
	lcd_data_queue(spi, (x2 >> 8));
	lcd_data_queue(spi, x2);

	lcd_cmd(spi, 0x2B);
	lcd_data_queue(spi, (y1 >> 8));
	lcd_data_queue(spi, y1);
	lcd_data_queue(spi, (y2));
	lcd_data_queue(spi, y2);

	lcd_cmd(spi, 0x2C);
}

/**
 * @fn void lcd_fill(spi_device_handle_t, uint16_t)
 * @brief fill the screen with a specific colour
 * 		  colours are defined in regarding header file
 *
 * @param spi
 * @param colour
 */
void lcd_fill(spi_device_handle_t spi, uint16_t colour) {
	uint16_t i, j;
	lcd_setaddress(spi, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	for (i = 0; i < LCD_WIDTH; i++) {
		for (j = 0; j < LCD_HEIGHT; j++) {
			lcd_draw_pixel(colour, spi);
		}
	}
}

void lcd_draw_circle(uint16_t xm, uint16_t ym, uint16_t r, uint16_t color,
		spi_device_handle_t spi) {
	int16_t f = 1 - r, ddF_x = 1, ddF_y = 0 - (2 * r);
	int16_t x = 0, y = r;

	lcd_draw_pixel_at(xm, ym + r, color, spi);
	lcd_draw_pixel_at(xm, ym - r, color, spi);
	lcd_draw_pixel_at(xm + r, ym, color, spi);
	lcd_draw_pixel_at(xm - r, ym, color, spi);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		lcd_draw_pixel_at(xm + x, ym + y, color, spi);
		lcd_draw_pixel_at(xm - x, ym + y, color, spi);
		lcd_draw_pixel_at(xm + x, ym - y, color, spi);
		lcd_draw_pixel_at(xm - x, ym - y, color, spi);
		lcd_draw_pixel_at(xm + y, ym + x, color, spi);
		lcd_draw_pixel_at(xm - y, ym + x, color, spi);
		lcd_draw_pixel_at(xm + y, ym - x, color, spi);
		lcd_draw_pixel_at(xm - y, ym - x, color, spi);
	}
}

void lcd_draw_pixel_at(uint16_t x, uint16_t y, uint16_t color,
		spi_device_handle_t spi) {
	if (!lcd_set_cursor(x, y, spi)) {
		lcd_draw_pixel(color, spi);
	}
}

void lcd_flush(spi_device_handle_t spi, uint16_t colour, uint16_t x, uint16_t y) {

	lcd_cmd(spi, 0x2A);
	lcd_data_queue(spi, (0 >> 8) & 0xFF);
	lcd_data_queue(spi, 0 & 0xFF);
	lcd_data_queue(spi, (319 >> 8) & 0xFF);
	lcd_data_queue(spi, 319 & 0xFF);

	lcd_cmd(spi, 0x2B);
	lcd_data_queue(spi, (0 >> 8) & 0xFF);
	lcd_data_queue(spi, 0 & 0xFF);
	lcd_data_queue(spi, (239 >> 8) & 0xFF);
	lcd_data_queue(spi, 239 & 0xFF);

	lcd_cmd(spi, 0x2C);
}

/**
 * @fn void lcd_start(spi_device_handle_t)
 * @brief start lcd display with default values
 * 		  given by the ILI9341 manual
 *
 * @param spi
 */
void lcd_start(spi_device_handle_t spi) {
	//Initialize non-SPI GPIOs
	gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
	gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
	gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

	//Reset the display
	gpio_set_level(PIN_NUM_RST, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(PIN_NUM_RST, 1);
	vTaskDelay(100 / portTICK_RATE_MS);

	lcd_cmd(spi, 0xCB);
	lcd_data_queue(spi, 0x39);
	lcd_data_queue(spi, 0x2C);
	lcd_data_queue(spi, 0x00);
	lcd_data_queue(spi, 0x34);
	lcd_data_queue(spi, 0x02);

	lcd_cmd(spi, 0xCF);
	lcd_data_queue(spi, 0x00);
	lcd_data_queue(spi, 0XC1);
	lcd_data_queue(spi, 0X30);

	lcd_cmd(spi, 0xE8);
	lcd_data_queue(spi, 0x85);
	lcd_data_queue(spi, 0x00);
	lcd_data_queue(spi, 0x78);

	lcd_cmd(spi, 0xEA);
	lcd_data_queue(spi, 0x00);
	lcd_data_queue(spi, 0x00);

	lcd_cmd(spi, 0xED);
	lcd_data_queue(spi, 0x64);
	lcd_data_queue(spi, 0x03);
	lcd_data_queue(spi, 0X12);
	lcd_data_queue(spi, 0X81);

	lcd_data_queue(spi, 0x20);

	lcd_cmd(spi, 0xC0); // Power control
	lcd_data_queue(spi, 0x23); // VRH[5:0]

	lcd_cmd(spi, 0xC1); // Power control
	lcd_data_queue(spi, 0x10); // SAP[2:0];BT[3:0]

	lcd_cmd(spi, 0xC5); // VCM control
	lcd_data_queue(spi, 0x3e);
	lcd_data_queue(spi, 0x28);

	lcd_cmd(spi, 0xC7); // VCM control2
	lcd_data_queue(spi, 0x86);

	lcd_cmd(spi, 0x36); // Memory Access Control
	lcd_data_queue(spi, 0x88); // C8

	lcd_cmd(spi, 0x3A);
	lcd_data_queue(spi, 0x55);

	lcd_cmd(spi, 0xB1);
	lcd_data_queue(spi, 0x00);
	lcd_data_queue(spi, 0x18);

	lcd_cmd(spi, 0xB6); // Display Function Control
	lcd_data_queue(spi, 0x08);
	lcd_data_queue(spi, 0x82);
	lcd_data_queue(spi, 0x27);

	lcd_cmd(spi, 0xF2); // 3Gamma Function Disable
	lcd_data_queue(spi, 0x00);

	lcd_cmd(spi, 0x26); // Gamma curve selected
	lcd_data_queue(spi, 0x01);

	lcd_cmd(spi, 0xE0); // Set Gamma
	lcd_data_queue(spi, 0x0F);
	lcd_data_queue(spi, 0x31);
	lcd_data_queue(spi, 0x2B);
	lcd_data_queue(spi, 0x0C);
	lcd_data_queue(spi, 0x0E);
	lcd_data_queue(spi, 0x08);
	lcd_data_queue(spi, 0x4E);
	lcd_data_queue(spi, 0xF1);
	lcd_data_queue(spi, 0x37);
	lcd_data_queue(spi, 0x07);
	lcd_data_queue(spi, 0x10);
	lcd_data_queue(spi, 0x03);
	lcd_data_queue(spi, 0x0E);
	lcd_data_queue(spi, 0x09);
	lcd_data_queue(spi, 0x00);

	lcd_cmd(spi, 0xE1); // Set Gamma
	lcd_data_queue(spi, 0x00);
	lcd_data_queue(spi, 0x0E);
	lcd_data_queue(spi, 0x14);
	lcd_data_queue(spi, 0x03);
	lcd_data_queue(spi, 0x11);
	lcd_data_queue(spi, 0x07);
	lcd_data_queue(spi, 0x31);
	lcd_data_queue(spi, 0xC1);
	lcd_data_queue(spi, 0x48);
	lcd_data_queue(spi, 0x08);
	lcd_data_queue(spi, 0x0F);
	lcd_data_queue(spi, 0x0C);
	lcd_data_queue(spi, 0x31);
	lcd_data_queue(spi, 0x36);
	lcd_data_queue(spi, 0x0F);

	lcd_cmd(spi, 0x11); // Sleep out
	vTaskDelay(120);
	lcd_cmd(spi, 0x2c);
	lcd_fill(spi, BLACK);

	lcd_cmd(spi, 0x29); // Display on
	lcd_cmd(spi, 0x2c);

	gpio_set_level(PIN_NUM_BCKL, 0);

}
