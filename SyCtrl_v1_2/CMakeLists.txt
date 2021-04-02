# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/components/lvgl_esp32_drivers 
						${CMAKE_CURRENT_LIST_DIR}/components/lvgl_esp32_drivers/lvgl_tft
						${CMAKE_CURRENT_LIST_DIR}/components/lvgl_esp32_drivers/lvgl_touch
						${CMAKE_CURRENT_LIST_DIR}/components/lvgl
						)				
						
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(SyCtrl_v1_2)

#C:/users/cleme/sylents/ESP32/SyCtrl_v1_2/