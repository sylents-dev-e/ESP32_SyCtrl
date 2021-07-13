/**
   @file    UART_DataLogging.ino
   @autor   Clemens K.
   @version 0.1
   @date    03/2021
   @brief   HX711 read via i2c-bus and write to Uart (serial stream)
            Calibration and Data stream in one file.
            1. make calibration
            2. start the measurement
            commands:
            f... force the measurement and write to SD
            s... stop
            c... change calbration value manually
            r... restart calibration (only in stopped mode --> "s")
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
  Serial.begin(115200);
  Serial.println("Starting UART...");

  /***************************** HX711 ************************************/
  startup();

  /***************************** END HX711 *********************************/
}

void loop() {
  static unsigned int counter = 1;
  char string[10] = "";
  char string2[10] = "";
  char outstring[64] = "";
  static boolean newDataReady = false;
  float i = 0.0;

  if (Serial.available()) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
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
      calibrate();
    }
    else if (inByte == 'c') changeSavedCalFactor();
    else if (inByte == 'x') SetStdFactor();
  }


  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
  if (flag == true) {
    /***************************** HX711 ************************************/

    if (LoadCell.getTareTimeoutFlag()) {
      Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
    }

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

    /***************************** END HX711 *********************************/
  }
}

void calibrate() {
  
  Serial.println("Start calibration with mass (type 'y') or set pre-defined calibration value and start measurent (type 'n').");

  boolean _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 'y') {
          calibrate_from_reference_weight();
        }
        else if (inByte == 'n') {
          SetStdFactor();
        }
      }
  }

  Serial.println("End calibration");
  Serial.println("'r' to re-calibrate.");
  Serial.println("'c' for manuel entry of calibration-val.");
  Serial.println("'x' set std calibration-val.");
  Serial.println("'s' START measurement.");
  Serial.println("'p' PAUSE measurement.");
  Serial.println("'t' to tare");
}

void changeSavedCalFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current Cal value: ");
  Serial.println(oldCalibrationValue);
  Serial.println("New Cal value, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;

  EEPROM.begin(512);
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Value ");
  Serial.print(newCalibrationValue);
  Serial.println(" saved as calibration value");

  Serial.println("end ");  // end set calibration value
  Serial.println("***");
}


void SetStdFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current Cal value: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Enter new Cal value, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }

  _resume = false;
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Value ");
  Serial.print(newCalibrationValue);
  Serial.println(" saved as calibration value");
  _resume = true;
}


void startup() {
  LoadCell.begin();
  long stabilisingtime = 2000;
  LoadCell.start(stabilisingtime);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(STD_CALIB_FACTOR);
    Serial.println("Startup + tare is complete");
  }
  while (!LoadCell.update());
  calibrate();
}

void calibrate_from_reference_weight(){
  
  Serial.println("Place load cell on stable surface and remove any load.");
  Serial.println("send 't' for tare offset.");

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

  Serial.println("Type in weight of known mass (in g):");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      Serial.print("Calibration mass: ");
      Serial.print(known_mass);
      Serial.println(" g");
      _resume = true;
      }
  }

  LoadCell.refreshDataSet();
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass);
  
  EEPROM.begin(512);
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Value ");
  Serial.print(newCalibrationValue);
  Serial.println(" saved as calibration value");
  _resume = true;

}
