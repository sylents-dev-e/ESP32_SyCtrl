/*
 * data_ILI9341.h
 *
 *  Created on: 12.05.2020
 *      Author: cleme
 */

#ifndef MAIN_DATA_ILI9341_H_
#define MAIN_DATA_ILI9341_H_

#include "main.h"
#include "lvgl/lvgl.h"
#include "lvgl_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

void guiTask(void *arg);
void parse_http_status(int32_t status, lv_obj_t *label3);

#ifdef __cplusplus
}
#endif


#endif /* MAIN_DATA_ILI9341_H_ */
