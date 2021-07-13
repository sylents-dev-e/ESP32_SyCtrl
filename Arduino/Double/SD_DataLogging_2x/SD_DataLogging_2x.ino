/**
   @file    SD_DataLogging_2x.ino
   @autor   Clemens K.
   @version 0.1
   @date    02/2021
   @brief   HX711 read via i2c-bus and write to SDcard (MMC-Mode)
            Dual Mode!
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
const int HX711_dout_1 = 19;
const int HX711_sck_1 = 18;
const int HX711_dout_2 = 26;
const int HX711_sck_2 = 25;
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1);
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2);

const int calVal1_eepromAdress = 0;
const int calVal2_eepromAdress = 4;
unsigned long t = 0;
const int delay_ms = 1000;
static boolean flag = false;
char filestring[64] = "";



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
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  /***************************** HX711 ************************************/
  startup(LoadCell_1, 1);
  startup(LoadCell_2, 2);

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
  float i, i2 = 0.0;

  if (Serial.available()) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell_1.tareNoDelay();
      LoadCell_2.tareNoDelay();
    }
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
      startup(LoadCell_1, 1);
      startup(LoadCell_2, 2);
      sd_start();
    }
    else if (inByte == 'c') {
      changeSavedCalFactor(LoadCell_1, 1);
      changeSavedCalFactor(LoadCell_2, 2); 
    }
  }


  if (LoadCell_2.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
  if (flag == true) {
    /***************************** HX711 ************************************/

    while (LoadCell_2.getTareTimeoutFlag() or LoadCell_1.getTareTimeoutFlag()) {
      Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
    }

    if (LoadCell_2.update() and LoadCell_1.update()) newDataReady = true;

    if (newDataReady) {
      if (millis() > t + delay_ms) {
        i = LoadCell_1.getData();
        i2 = LoadCell_2.getData();
        Serial.print("Load_cell 1 output val: ");
        Serial.print(i);
        Serial.println(" g");
        Serial.print("Load_cell 2 output val: ");
        Serial.print(i2);
        Serial.println(" g");
        newDataReady = false;
        int size_val = snprintf(NULL, NULL, "%d;%0.2f;%0.2f\n", counter * delay_ms, i, i2);
        snprintf(outstring, size_val + 1, "%d;%0.2f;%0.2f\n", counter * delay_ms, i, i2);
        appendFile(SD_MMC, filestring, outstring);
        counter++;

        Serial.printf("Time elapsed: %d ms\n", counter * delay_ms);
        
        t = millis();
      }
    }

    /***************************** END HX711 *********************************/

    /***************************** SD CARD ************************************/

    /***************************** END SD CARD *********************************/
  }
}

void calibrate(HX711_ADC LoadCell, uint8_t cell_num) {
  int address = 42;
  Serial.println("***");
  Serial.printf("Start calibration: regarding Loadcell %d\n", cell_num);
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
      Serial.printf("Tare complete regarding Loadcell %d\n", cell_num);
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (in g) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      //if (known_mass != 0) {
      Serial.print("Known mass is: ");
      Serial.print(known_mass);
      Serial.printf(" g regarding Loadcell %d\n", cell_num);
      _resume = true;
      //}
    }
  }

  LoadCell.refreshDataSet(); 
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); 

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM address ");
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        if(cell_num == 1){
          address = calVal1_eepromAdress;
        }
        else if(cell_num == 2){
          address = calVal2_eepromAdress;
        }
        else address = 42;
        
        EEPROM.begin(512);
        EEPROM.put(address, newCalibrationValue);
        EEPROM.commit();
        EEPROM.get(address, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(address);
        _resume = true;

      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.printf("End calibration regarding Loadcell %d\n", cell_num);
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}

void changeSavedCalFactor(HX711_ADC LoadCell, uint8_t cell_num) {
  int address = 42;
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
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

  if(cell_num == 1){
    address = calVal1_eepromAdress;
  }
  else if(cell_num == 2){
    address = calVal2_eepromAdress;
  } 
  _resume = false;
  Serial.print("Save this value to EEPROM address ");
  Serial.print(address);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        EEPROM.begin(512);
        EEPROM.put(address, newCalibrationValue);
        EEPROM.commit();
        EEPROM.get(address, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(address);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  Serial.printf("End change calibration value regarding Loadcell %d\n", cell_num);
  Serial.println("***");
}

void startup(HX711_ADC LoadCell, uint8_t cell_num) {
  LoadCell.begin();
  long stabilisingtime = 2000;
  LoadCell.start(stabilisingtime);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.printf("Tare timeout, check MCU>HX711 wiring and pin designations regarding Loadcell %d\n", cell_num);
  }
  else {
    LoadCell.setCalFactor(1.0);
    Serial.printf("Startup + tare is complete regarding Loadcell %d\n", cell_num);
  }
  while (!LoadCell.update());
  calibrate(LoadCell, cell_num);
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
    Serial.println("No SD_MMC card attached");
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

  Serial.println("enter current time e.g. 1211 (for .csv file name-timestamp).");

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
