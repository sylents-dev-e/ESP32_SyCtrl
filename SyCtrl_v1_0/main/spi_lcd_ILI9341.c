/*
 * spi_lcd_ILI9341.c
 *
 *  Created on: 24.04.2020
 *      Author: cleme
 */

#include "spi_lcd_ILI9341.h"
#include "font.h"



/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, uint8_t *data, uint8_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;               //Len is in bytes, transaction length is in bits.
    t.tx_data[0] =(uint8_t) *data,
  //  t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_queue_trans(spi, &t, portMAX_DELAY);
    //ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

uint32_t lcd_get_id(spi_device_handle_t spi)
{
    //get_id cmd
    lcd_cmd(spi, 0x04);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8*3;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;

    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*)t.rx_data;
}

//Initialize the display
void lcd_init(spi_device_handle_t spi)
{
    uint8_t cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;

    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    //detect LCD type
    uint32_t lcd_id = lcd_get_id(spi);
    int lcd_detected_type = 0;
    int lcd_type =0;

    printf("LCD ID: %08X\n", lcd_id);
    if ( lcd_id == 0 ) {
        //zero, ili
        lcd_detected_type = LCD_TYPE_ILI;
        printf("ILI9341 detected.\n");
    } else {
        // none-zero, ST
        lcd_detected_type = LCD_TYPE_ST;
        printf("ST7789V detected.\n");
    }

#ifdef CONFIG_LCD_TYPE_AUTO
    lcd_type = lcd_detected_type;
#elif defined( CONFIG_LCD_TYPE_ST7789V )
    printf("kconfig: force CONFIG_LCD_TYPE_ST7789V.\n");
    lcd_type = LCD_TYPE_ST;
#elif defined( CONFIG_LCD_TYPE_ILI9341 )
    printf("kconfig: force CONFIG_LCD_TYPE_ILI9341.\n");
    lcd_type = LCD_TYPE_ILI;
#endif
    if ( lcd_type == LCD_TYPE_ST ) {
        printf("LCD ST7789V initialization.\n");
        lcd_init_cmds = st_init_cmds;
    } else {
        printf("LCD ILI9341 initialization.\n");
        lcd_init_cmds = ili_init_cmds;
    }

  //Send all the commands
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(spi, lcd_init_cmds[cmd].cmd);
        lcd_data(spi,lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        if (lcd_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }
    //Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 0);

}

uint8_t lcd_set_cursor_y(uint16_t y, spi_device_handle_t spi)
{

    if( y >= LCD_HEIGHT )
    {
        return EXIT_FAILURE;
    }

    lcd_cmd(spi,  0x2A);
    lcd_data(spi,(uint8_t*) (y >> 8), 2);
    lcd_data(spi,(uint8_t*) (y & 0xFF), 2);
    lcd_cmd(spi,  0x2c);

    return EXIT_SUCCESS;
}

uint8_t lcd_set_cursor_x(uint16_t x, spi_device_handle_t spi)
{
    if( x >= LCD_WIDTH )
    {
        return EXIT_FAILURE;
    }
    lcd_cmd(spi,  0x2B);
    lcd_data(spi,(uint8_t*) (x >> 8), 2);
    lcd_data(spi, (uint8_t*)(x & 0xFF), 2);
    lcd_cmd(spi,  0x2c);

    return EXIT_SUCCESS;
}

uint8_t lcd_set_cursor(uint16_t x, uint16_t y, spi_device_handle_t spi)
{
    if( lcd_set_cursor_x(x,spi) || lcd_set_cursor_y(y,spi) )
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


uint8_t lcd_draw_pixel(uint16_t color, spi_device_handle_t spi)
{
    lcd_data(spi,(uint8_t*) (color >> 8), 2);
    lcd_data(spi,(uint8_t*)(color & 0xFF), 2);

    return EXIT_SUCCESS;
}

/*******************************************************************************
 * Fuellen des gesamten Displays mit einer frei waehlbaren Farbe
 */

void lcd_fill(uint16_t bg_color,spi_device_handle_t spi)
{
    uint16_t width = LCD_WIDTH, height = LCD_HEIGHT;

    if( lcd_set_cursor(0,0,spi) )
    {
        return;
    }

    while(height--)
    {
        while(width--)
        {
            lcd_draw_pixel(bg_color,spi);
        }
        width = LCD_WIDTH;
    }
}

void lcd_draw_string (uint16_t x, uint16_t y, const char *pS, uint16_t fg_color, uint16_t bg_color, spi_device_handle_t spi)
{
    uint16_t lIndex, k;

    while(*pS)
    {
        /* index the width information of character <c> */
        lIndex = 0;
        for(k=0; k<(*pS - ' '); k++)
        {
            lIndex += ((font[lIndex]) << 1) + 1;
        }

        /* draw character */
        lcd_draw_char(x, y, lIndex, fg_color, bg_color,spi);

        /* move the cursor forward for the next character */
        x += font[lIndex];

        /* next charachter */
        pS++;
    }
}

void lcd_draw_char (uint16_t x, uint16_t y, uint16_t fIndex, uint16_t fg_color, uint16_t bg_color,spi_device_handle_t spi)
{
    uint8_t j, k, i;

    for(j=0; j < font[fIndex]; j++) // variable character width
    {
        lcd_set_cursor(x + font[fIndex] - j, y, spi);

        for(k=0; k<FONT_HEIGHT; k++) // _ bytes per character (height)
        {
            for(i=0; i<8; i++)
            {
                if( font[ (fIndex + ((font[fIndex]) << 1)) - (j<<1) - k ] & (0x80 >> i) )
                {
                    lcd_draw_pixel(fg_color, spi);
                }
                else
                {
                    lcd_draw_pixel(bg_color, spi);
                }
            }
        }
    }
}

void lcd_start (spi_device_handle_t spi)
{
    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);


    lcd_cmd(spi,0xCF);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0X83,1);
    lcd_data(spi,(uint8_t*) 0X30,1);

    lcd_cmd(spi, 0xED);
    lcd_data(spi,(uint8_t*) 0x64,1);
    lcd_data(spi,(uint8_t*) 0x03,1);
    lcd_data(spi,(uint8_t*) 0X12,1);
    lcd_data(spi,(uint8_t*) 0X81,1);

    lcd_cmd(spi,0xE8);
    lcd_data(spi,(uint8_t*) 0x85,1);
    lcd_data(spi,(uint8_t*) 0x01,1);
    lcd_data(spi,(uint8_t*) 0x79,1);

    lcd_cmd(spi, 0xCB);
    lcd_data(spi, (uint8_t*) 0x39,1);
    lcd_data(spi, (uint8_t*) 0x2C,1);
    lcd_data(spi, (uint8_t*) 0x00,1);
    lcd_data(spi, (uint8_t*) 0x34,1);
    lcd_data(spi, (uint8_t*) 0x02,1);

    lcd_cmd(spi, 0xF7);
    lcd_data(spi, (uint8_t*)0x20,1);

    lcd_cmd( spi, 0xEA);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0x00,1);

    lcd_cmd(spi, 0xC0); // Power control
    lcd_data(spi,(uint8_t*) 0x26,1);//VRH[5:0]

    lcd_cmd(spi,0xC1); // Power control
    lcd_data(spi, (uint8_t*)0x11,1);//SAP[2:0];BT[3:0]

    lcd_cmd( spi, 0xC5); // VCM control
    lcd_data(spi,(uint8_t*) 0x35,1);
    lcd_data(spi, (uint8_t*)0x3E,1);

    lcd_cmd(spi, 0xC7); // VCM control2
    lcd_data(spi,(uint8_t*) 0xBE,1);

    lcd_cmd(spi, 0x36); // Memory Access Control
    lcd_data(spi, (uint8_t*)0x28,1); // C8

    lcd_cmd(spi, 0x3A);
    lcd_data(spi, (uint8_t*)0x55,1);

    lcd_cmd(spi,0xB1);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0x1B,1);

    lcd_cmd(spi, 0xF2);
    lcd_data(spi, (uint8_t*)0x08,1);

    lcd_cmd(spi, 0x26);
    lcd_data(spi, (uint8_t*)0x01,1);

    lcd_cmd( spi, 0xF2); // 3Gamma Function Disable
    lcd_data(spi,(uint8_t*) 0x08,1);

    lcd_cmd(spi, 0x26); // Gamma curve selected
    lcd_data(spi,(uint8_t*) 0x01,1);

    lcd_cmd(spi, 0xE0); // Set Gamma
    lcd_data(spi,(uint8_t*) 0x1F,1);
    lcd_data(spi,(uint8_t*) 0x1A,1);
    lcd_data(spi,(uint8_t*) 0x18,1);
    lcd_data(spi,(uint8_t*) 0x0A,1);
    lcd_data(spi,(uint8_t*) 0x0F,1);
    lcd_data(spi,(uint8_t*) 0x06,1);
    lcd_data(spi,(uint8_t*) 0x45,1);
    lcd_data(spi,(uint8_t*) 0X87,1);
    lcd_data(spi,(uint8_t*) 0x32,1);
    lcd_data(spi,(uint8_t*) 0x0A,1);
    lcd_data(spi,(uint8_t*) 0x07,1);
    lcd_data(spi,(uint8_t*) 0x02,1);
    lcd_data(spi,(uint8_t*) 0x07,1);
    lcd_data(spi,(uint8_t*) 0x05,1);
    lcd_data(spi,(uint8_t*) 0x00,1);

    lcd_cmd(spi, 0xE1); // Set Gamma
    lcd_data(spi, (uint8_t*)0x00,1);
    lcd_data(spi,(uint8_t*) 0x25,1);
    lcd_data(spi,(uint8_t*) 0x27,1);
    lcd_data(spi, (uint8_t*)0x05,1);
    lcd_data(spi,(uint8_t*) 0x10,1);
    lcd_data(spi,(uint8_t*) 0x09,1);
    lcd_data(spi,(uint8_t*) 0x3A,1);
    lcd_data(spi,(uint8_t*) 0x78,1);
    lcd_data(spi,(uint8_t*) 0x4D,1);
    lcd_data(spi,(uint8_t*) 0x05,1);
    lcd_data(spi,(uint8_t*) 0x18,1);
    lcd_data(spi,(uint8_t*) 0x0D,1);
    lcd_data(spi,(uint8_t*) 0x38,1);
    lcd_data(spi,(uint8_t*) 0x3A,1);
    lcd_data(spi,(uint8_t*) 0x1F,1);

    lcd_cmd( spi, 0x2A);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0xEF,1);

    lcd_cmd( spi, 0x2B);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0x00,1);
    lcd_data(spi,(uint8_t*) 0x01,1);
    lcd_data(spi,(uint8_t*) 0x3F,1);

    lcd_cmd( spi, 0x2C);

    lcd_cmd( spi, 0xB7);
    lcd_data(spi,(uint8_t*) 0x07,1);

    lcd_cmd( spi, 0xB6); // Display Function Control
    lcd_data(spi,(uint8_t*) 0x0A,1);
    lcd_data(spi,(uint8_t*) 0x82,1);
    lcd_data(spi,(uint8_t*) 0x27,1);
    lcd_data(spi,(uint8_t*) 0x00,1);

    lcd_cmd(spi, 0x11); // Sleep out
    vTaskDelay(100 / portTICK_RATE_MS);
 // lcd_fill(0xFFE0);

    lcd_cmd(spi, 0x29); // Display on

    gpio_set_level(PIN_NUM_BCKL, 0);

}

void send_lines(spi_device_handle_t spi, int ypos, uint16_t *linedata)
{
    esp_err_t ret;
    int x;
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans[6];

    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            //Even transfers are commands
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            //Odd transfers are data
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;           //Column Address Set
    trans[1].tx_data[0]=0;              //Start Col High
    trans[1].tx_data[1]=0;              //Start Col Low
    trans[1].tx_data[2]=(320)>>8;       //End Col High
    trans[1].tx_data[3]=(320)&0xff;     //End Col Low
    trans[2].tx_data[0]=0x2B;           //Page address set
    trans[3].tx_data[0]=ypos>>8;        //Start page high
    trans[3].tx_data[1]=ypos&0xff;      //start page low
    trans[3].tx_data[2]=(ypos+PARALLEL_LINES)>>8;    //end page high
    trans[3].tx_data[3]=(ypos+PARALLEL_LINES)&0xff;  //end page low
    trans[4].tx_data[0]=0x2C;           //memory write
    trans[5].tx_buffer=linedata;        //finally send the line data
    trans[5].length=320*2*8*PARALLEL_LINES;          //Data length, in bits
    trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag

    //Queue all transactions.
    for (x=0; x<6; x++) {
        ret=spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }

    //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
    //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
    //finish because we may as well spend the time calculating the next line. When that is done, we can call
    //send_line_finish, which will wait for the transfers to be done and check their status.
}

