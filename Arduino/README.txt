.csv-file: 
[ms];[g]


How to use:

1. Insert SD-card into the ESP32-slot.
Connect the HX711 to the ESP32(GND, 3v3, Data and Clock).
-> DT to IO19
-> SCK to IO18

2. Start the Arduino IDE and load the "SD_DataLogging"-file.
Check the prerequisites: 
https://github.com/espressif/arduino-esp32

- Tools->Board:____->Boards Manager: search for esp32 and install lib (from espressif).
- Tools->Board:____->ESP32 Arduino->ESP32 Wrover Module
- Tools->Library Manager: search for "hx711_adc", choose the one from Olav.
- Connect the board.
- Correct Board is selected in Tools (ESP32 Wrover Module).
- Select the proper COM-Port of the USB (in Tools).

3. Press Ctrl+Shift+M to start the serial monitor.

4. Compile and flash the Sketch to the device. Click on the "arrow-the-right"-button.
In the lower section of the window the flashing status will appear. Check that no errors occur.

5. In the Serial-Monitor: follow the instructions that pop up!!! (typically start with pressing 't' for tare)

6. Enter the mass in a valid digit representation.

7. EEPROM store the calib. In doubt, press 'y' ;-)

8. Enter time e.g. 1211 for 12:11 (to be added to filename).

9. Press 'f' to start the operation: timestamp and data will be written to file
IMPORTANT: to change the intervall simply change variable //delay_ms// in the code [ms].

10. Press 's' to stop.

11. Eject the SD-card. (Once ejected, the program sometimes tends to refuse writing to the card again as there is a bug within the API).
IMPORTANT: simply press the EN-button on the ESP32 board to restart the program after re-insterting the SD. This reboots the 
ESP32.  


For the dual-mode:

Adapt the system as there are 2 seperate I2C bus-systems. Follow instructions from serial-monitor. ;-)
Measurement station 1:
-> DT to IO19
-> SCK to IO18


Measurement station 2:
-> DT to IO26
-> SCK to IO25
