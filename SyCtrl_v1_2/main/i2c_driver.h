/*
 * i2c_driver.h
 *
 *  Created on: 08.04.2020
 *      Author: cleme
 */

#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#define SDA_PIN 33 //not defined
#define SCL_PIN 32 //not defined

#ifdef __cplusplus
extern "C" {
#endif
void i2c_master_init();
#ifdef __cplusplus
}
#endif

#endif /* __I2C_DRIVER_H__ */
