/**
   @file    SD_DataLogging_old.ino
   @autor   Clemens K.
   @version 0.2
   @date    02/2021
   @brief   HX711 read via i2c-bus and write to SDcard (MMC-Mode)
            commands:
            f... force the measurement and write to SD
            s... stop
            c... change calbration value manually
            r... restart calibration (only in stopped mode --> "s")
            t... tare
   @note    not completely configured yet! Follow the commands shown in serial output
*/

#include "FS.h"
#include "SD_MMC.h"
#include <HX711_ADC.h>
#include <EEPROM.h>

/* globals */
const int HX711_dout = 19;
const int HX711_sck = 18;
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;
unsigned long t = 0;
const int delay_ms = 100;          // DELAY
static boolean flag = false;
char filestring[64] = "";
const float toNm = 1.486215 / 1000;
const float StdCalValue = 396.42;

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);
  File file;
  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  //  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  /***************************** HX711 ************************************/
  startup();

  /***************************** END HX711 *********************************/

  /***************************** SD CARD ************************************/
  sd_start();
  /***************************** END SD CARD *********************************/
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
    else if (inByte == 's') {
      Serial.println("Measurement stopped.");
      //file.close();
      SD_MMC.end();
      flag = false;
      counter = 0;
    }
    else if (inByte == 'f') {
      Serial.println("Measurement started.");
      flag = true;
    }
    else if (inByte == 'r') {
      Serial.println("Measurement re-started.");

      startup();
      sd_start();
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
      if (millis() > t + delay_ms) {
        i = LoadCell.getData();
        //        Serial.print("Load_cell output val: ");
        //        Serial.print(i*toNm);
        //        Serial.println(" Nm");                               // print Torque
        // Serial.print(i);
        // Serial.print(" g -->");// print weight
        Serial.print(i*toNm);
        Serial.println(" Nm");// print torque
        newDataReady = false;

        int size_val = snprintf(NULL, NULL, "%d;%.2f\n", counter * delay_ms, i);
        snprintf(outstring, size_val + 1, "%d;%.2f\n", counter * delay_ms, i);
        appendFile(SD_MMC, filestring, outstring);

        counter++;

        // Serial.printf("%d ms\n", counter * delay_ms);       // print Delay
        t = millis();
      }
    }

    /***************************** END HX711 *********************************/

    /***************************** SD CARD ************************************/

    //t = millis();

    /***************************** END SD CARD *********************************/
  }
}

void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
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

  Serial.println("Get calibration value from EEPROM storage? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();

      if (inByte == 'n') {

        Serial.println("Re-calibrating.");
        Serial.println("Put calibration mass on LC and");
        Serial.println("type in weight (in g):");

        float known_mass = 0;
        _resume = false;
        while (_resume == false) {
          LoadCell.update();
          if (Serial.available() > 0) {
            known_mass = Serial.parseFloat();
            //if (known_mass != 0) {
            Serial.print("Calibration mass: ");
            Serial.print(known_mass);
            Serial.println(" g");
            _resume = true;
            //}
          }
        }

        LoadCell.refreshDataSet();
        float newCalibrationValue = LoadCell.getNewCalibration(known_mass);

        Serial.print("Calibration value: ");
        Serial.print(newCalibrationValue);
        Serial.println(", use calibration value (calFactor).");
        Serial.print("Save to EEPROM x");
        Serial.print(calVal_eepromAdress);
        Serial.println("? y/n");

        _resume = false;
        while (_resume == false) {
          if (Serial.available() > 0) {
            char inByte2 = Serial.read();
            if (inByte2 == 'y') {
              EEPROM.begin(512);
              EEPROM.put(calVal_eepromAdress, newCalibrationValue);
              EEPROM.commit();
              EEPROM.get(calVal_eepromAdress, newCalibrationValue);
              Serial.print("Value ");
              Serial.print(newCalibrationValue);
              Serial.print(" saved to EEPROM address: ");
              Serial.println(calVal_eepromAdress);
              _resume = true;

            }
            else if (inByte2 == 'n') {
              Serial.println("Value not saved to EEPROM");
              _resume = true;
            }
          }
        }
      }
      if (inByte == 'y') {
        float newCalibrationValue_GetfromEEPROM = 0;

        EEPROM.get(calVal_eepromAdress, newCalibrationValue_GetfromEEPROM);
        Serial.print("From EEPROM Calibration: ");
        Serial.println(newCalibrationValue_GetfromEEPROM);
        LoadCell.setCalFactor(newCalibrationValue_GetfromEEPROM);
      }

      _resume = true;
    }

  }

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("'r' to re-calibrate.");
  Serial.println("'c' for manuel entry of calibration-val.");
  Serial.println("'x' set std calibration-val.");
  Serial.println("***");
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
  Serial.print("Save to EEPROM x");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        EEPROM.begin(512);
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
        EEPROM.commit();
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM x");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("...");   // not saved to eeprom
        _resume = true;
      }
    }
  }
  Serial.println("end ");  // end set calibration value
  Serial.println("***");
}


void SetStdFactor() {
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
  newCalibrationValue = StdCalValue;

  _resume = false;
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Value ");
  Serial.print(newCalibrationValue);
  Serial.print(" saved to EEPROM x");
  Serial.println(calVal_eepromAdress);
  _resume = true;
  Serial.println("end ");  // end set calibration value
  Serial.println("***");
}




void startup() {
  LoadCell.begin();
  long stabilisingtime = 2000;
  LoadCell.start(stabilisingtime);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(1.0);
    Serial.println("Startup + tare is complete");
  }
  while (!LoadCell.update());
  calibrate();
}

void sd_start() {
  boolean console_stop = false;
  int timestamp = 0;

  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card");
    return;
  }

  Serial.print("SD_MMC Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN. EXIT");
    exit(0);
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  Serial.println("enter file UID e.g. time 1211.");

  console_stop = false;
  while (console_stop == false) {
    if (Serial.available() > 0) {
      Serial.setTimeout(100000);
      timestamp = Serial.parseInt();
      console_stop = true;
    }
  }

  snprintf(filestring, sizeof(filestring), "/data_%d.csv", timestamp);
  Serial.println(filestring);
  writeFile(SD_MMC, filestring, NULL);

  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
  Serial.println("Type 'f' to start and 's' to stop.");

}
