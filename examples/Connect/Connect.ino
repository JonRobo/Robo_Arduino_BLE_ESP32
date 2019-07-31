#include "Robo_BLE.h"

#define Robo_Name  "Robo_XYZ"


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

void loop() {
  delay(2000);
}
