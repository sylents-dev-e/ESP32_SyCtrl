/**
   @file    HX711_only_Calibration.ino
   @autor   Clemens K.
   @version 0.1
   @date    03/2021
   @brief   HX711 calib via i2c-bus and write to Uart (serial stream)
            commands:
            c... change calbration value manually
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


void setup() {
  Serial.begin(115200);
  Serial.println("Starting UART...");

  LoadCell.begin();
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(1.0);
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  calibrate();
}

void loop() {

  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
    else if (inByte == 'r') calibrate(); 
    else if (inByte == 'c') changeSavedCalFactor(); 
  }

  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

}

void calibrate() {
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }
    }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass in g (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell.refreshDataSet();
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass);


  EEPROM.begin(512);
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Calib-Value ");
  Serial.print(newCalibrationValue);
  Serial.print(" saved.");
  Serial.println("End calibration");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
}

void changeSavedCalFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  EEPROM.begin(512);
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Calib-Value ");
  Serial.print(newCalibrationValue);
  Serial.print(" saved.");

}
