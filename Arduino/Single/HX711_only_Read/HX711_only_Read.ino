/**
   @file    HX711_only_Read.ino
   @autor   Clemens K.
   @version 0.1
   @date    03/2021
   @brief   HX711 read via i2c-bus and write to Uart (serial stream)
            after calib-value is entered manually
            commands:
            s... start measurement
            p... pause measurement 
            r... restart calibration
            t... tare
   @note    
*/

#include <HX711_ADC.h>
#include <EEPROM.h>

/* globals */
const int HX711_dout = 19;
const int HX711_sck = 18;
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;
unsigned long t = 0;
static boolean flag = false;

/* USER DEFINES */
const int DELAY_MS = 100;       
const float TO_NM_FACTOR = 1.486215 / 1000;
const float STD_CALIB_FACTOR = 396.42;


void setup() {
  boolean _resume = false;
  Serial.begin(115200);
  Serial.println("Starting UART...");
  startup();
}

void loop() {
  static boolean newDataReady = 0;
  static unsigned int counter = 1;
  float i = 0.0;

  if (Serial.available()) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell.tareNoDelay();
    }
    else if (inByte == 'p') {
      Serial.println("Measurement stopped.");
      flag = false;
      counter = 0;
    }
    else if (inByte == 's') {
      Serial.println("Measurement started.");
      flag = true;
    }
    else if (inByte == 'r') {
      Serial.println("Measurement re-started.");
      startup();
    }
  }

  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
  if (flag == true) {
    if (LoadCell.update()) newDataReady = true;

    if (newDataReady) {
      if (millis() > t + DELAY_MS) {
         i = LoadCell.getData();
         //        Serial.print("Load_cell output val: ");
         //        Serial.print(i*TO_NM_FACTOR);
         //        Serial.println(" Nm");                               // print Torque
         // Serial.print(i);
         // Serial.print(" g -->");// print weight
         Serial.print(i*TO_NM_FACTOR);
         Serial.println(" Nm");// print torque
         newDataReady = false;
  
         counter++;
  
         // Serial.printf("%d ms\n", counter * DELAY_MS);       // print Delay
         t = millis();
      }
    }
  }
}

void startup(){
  LoadCell.begin();
  boolean _resume = false;
  float newCalibrationValue;
  
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        _resume = true;
      }
    }
  }
  

  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(newCalibrationValue);
    Serial.println("Startup is complete");
    Serial.println("'s' to start, 'p' to pause, 'r' to restart with new calib-value, 't' to tare.");
  }
}
