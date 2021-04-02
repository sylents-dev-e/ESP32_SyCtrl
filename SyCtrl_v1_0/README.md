## SyCtrl_v1_1 ESP32 Motor Control Application *deprecated backup version*

Controlling PWM-output signal via duty cycle modification (frequency is always set to 50Hz). See @Parameters section.
#### Display
![alt text](https://github.com/sylents-dev-e/sylents-controller-unit/blob/master/ESP32/SyCtrl_v1_0/display.jpg)

#### Example: 2200ms high-time
![alt text](https://github.com/sylents-dev-e/sylents-controller-unit/blob/master/ESP32/SyCtrl_v1_0/pwm_2200ms.PNG)

## How to use

* Build and flash the project eighter with Eclipse or via bare-Metal command-line method. 
* Modify the BLDC motor's duty-cycle modes (see section @Modes) when pressing UP and DOWN buttons on the Joystick Board
* Emergency break when pressing LEFT button (pwm set to minimum)
* Modify the servo motor's duty-cycle when moving the Joystick to the LEFT or RIGHT

## Modes

* 850µs (*1)
* 920µs (*2)
* 1400µs (*3)
* 1800µs (*4)
* 2200µs (*5)

## Parameters
#### PWM duty cycle:
Via defines in main/pwm_driver.h

## SW Architecture

* specified in "System Architecture Sylents Control"-file in Google-Drive/Dev-E/Embedded-Systems

## Documentation
#### Getting Started Raw
https://docs.espressif.com/projects/esp-idf/en/release-v3.2/get-started/index.html#install-the-required-python-packages
#### Getting Started Eclipse
https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md

## HW/SW Information
#### ESP-IDF
https://docs.espressif.com/projects/esp-idf/en/release-v3.2/index.html

#### ESP32-WROVER-KIT µController
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-wrover-kit.html

#### (not in use) ESP32-WROOM-32D µController
https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf

#### Eclipse
https://www.eclipse.org/downloads/packages/
Choose C/C++ Developement IDE

## Debug Information
https://docs.espressif.com/projects/esp-idf/en/release-v3.2/api-guides/jtag-debugging/index.html

## BUGS

* 2 Column display

## Bugfixes

* WDT Config refering the idle state 27/03/2020
* Button Debouncing 02/04/2020
* down button is not working --> conflicting semaphores in pwm configuration 06/04/2020
* UP/DOWN count switch, buttons --> xQueueOverwriteFromISR has to be used in ISR 08/04/2020

*Edited by Clemens Környefalvy*
