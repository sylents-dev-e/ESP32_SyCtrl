## SyCtrl_v1_1 ESP32 Motor Control Application *updated recently*

Controlling PWM-output signal via duty cycle modification (frequency is always set to 50Hz). See @Parameters section.

![alt text](https://github.com/sylents-dev-e/sylents-controller-unit/blob/master/ESP32/SyCtrl_v1_1/ILI9341.jpg)

## How to use

* Build and flash the project eighter with Eclipse or via bare-Metal command-line method. 
* Modify the BLDC motor's duty-cycle modes (see section @Modes) when pressing UP and DOWN buttons on the Joystick Board
* Emergency break when pressing LEFT button (pwm set to minimum)
* Connect the Smartphone App to the AP from the µC, control the motor through the Smartphone
* Modify the servo motor's duty-cycle when moving the Joystick to the LEFT or RIGHT

## Pin-Config on ESP32 Wrover Kit v4

* PWM-Throttle: **IO_32**
* PWM-Steering: **IO_13**
* Button-Emergency: **IO_4**
* Button-UP: **IO_35**
* Button-DOWN: **IO_34**
* ADC Input: **IO_36**
* SPI_MISO medium display: **IO_25**
* SPI_MOSI medium display: **IO_23**
* SPI_CLK: **IO_19**
* SPI_CS: **IO_22**
* I2C small display SDA_PIN: **not in use**
* I2C small display SCL_PIN: **not in use**

## Modes

* 850µs (*1)
* 920µs (*2)
* 1400µs (*3)
* 1800µs (*4)
* 2200µs (*5)

## Parameters
#### PWM duty cycle:
Via defines in main/pwm_driver.h

## Task Memory Infos

* documented in /documents/task_info.txt
* enable DEBUG_TASK_STACK in main.h to enable realtime data to serial output

## SW Architecture

* specified in "System Architecture Sylents Control"-file in Google-Drive/Dev-E/Embedded-Systems

## Documentation
#### Getting Started Raw
https://docs.espressif.com/projects/esp-idf/en/release-v3.2/get-started/index.html#install-the-required-python-packages
#### Getting Started Eclipse
https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md
#### Build System
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html
#### Display Driver 
https://github.com/littlevgl/
https://docs.lvgl.io/en/html/index.html

## HW/SW Information
#### ESP-IDF
https://docs.espressif.com/projects/esp-idf/en/release-v3.2/index.html

#### ESP32-WROVER-KIT µController
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-wrover-kit.html

#### ILI9341 Display 
https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf

#### (not in use) ESP32-WROOM-32D µController
https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf

#### Eclipse
https://www.eclipse.org/downloads/packages/
Choose C/C++ Developement IDE

## Debug Information
https://docs.espressif.com/projects/esp-idf/en/release-v3.2/api-guides/jtag-debugging/index.html

## BUGS

* ADC potentiometer middle position is not exactly at the half of the voltage (Software solution for handling that?)
* Several Error handling/messaging, see FIXME points
* Interrupt button routines worked more solid than permanent task query of digital pin
* WTD Implementation of control

## Bugfixes

* WDT Config refering the idle state
* Button Debouncing
* down button is not working --> conflicting semaphores in pwm configuration
* UP/DOWN count switch, buttons --> xQueueOverwriteFromISR has to be used in ISR 
* Switchable modes between button ISR mode and "normal" pin-reading
* Shared Memory secure handling in Message Queue between APP<--PWM-->Buttons
* Task Sync between between APP<--PWM-->Buttons
* ADC - App steering control: ADC values are always set, http data not!

## Backlog Bugs

* 128x32: 2 Column display 
* Flushing old display values (320x240 display) --> self-made custom driver (deprecated)

#### Example: 2200ms high-time
![alt text](https://github.com/sylents-dev-e/sylents-controller-unit/blob/master/ESP32/SyCtrl_v1_1/pwm_2200ms.PNG)

#### OLD Display (deprecated)
![alt text](https://github.com/sylents-dev-e/sylents-controller-unit/blob/master/ESP32/SyCtrl_v1_1/display.jpg)



*Edited by Clemens Környefalvy*
