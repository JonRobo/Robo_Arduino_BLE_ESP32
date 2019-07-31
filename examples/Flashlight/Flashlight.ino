#include "Robo_BLE.h"

#define Robo_Name  "Robo_XYZ" // change to match the Robo name you wish to connect to
#define Button_ID 1

bool light_status = 0;

// Requires RGB LED and Button

// Create Robo_BLE Instance
Robo_BLE RW = Robo_BLE();

void setup() {
  Serial.begin(115200);
  
  if(RW.init(Robo_Name)){
    Serial.println("Robo Connection Success!");
    Serial.println(RW.get_firmware_version());
    RW.set_button_trigger(1,1,Button_ID);
  }
  else Serial.println("Robo Connection Failure!");
}

void flashlight(){
  if(RW.monitor_id(Button_ID)){
    RW.set_button_trigger(1,1,Button_ID);
    light_status = !light_status;
    if(light_status) RW.RGB_WHITE(1);
    else RW.RGB_OFF(1);
  }
}


void loop() {
  flashlight();
  delay(100);
}
