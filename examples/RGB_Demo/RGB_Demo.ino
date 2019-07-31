#include "Robo_BLE.h"

#define Robo_Name  "Robo_XYZ" // change to match the Robo name you wish to connect to


// Create Robo_BLE Instance
Robo_BLE RW = Robo_BLE();

void setup() {
  Serial.begin(115200);
  
  if(RW.init(Robo_Name)){
    Serial.println("Robo Connection Success!");
    Serial.println(RW.get_firmware_version());
  }
  else Serial.println("Robo Connection Failure!");
}

void RGB_Demo(){
  RW.RGB(255,51,180,1);
  delay(2000);
  RW.RGB_RED(1);
  delay(2000);
  RW.RGB_GREEN(1);
  delay(2000);
  RW.RGB_BLUE(1);
  delay(2000);
  RW.RGB_OFF(1);
  delay(2000);
}


void loop() {
  RGB_Demo();
}
