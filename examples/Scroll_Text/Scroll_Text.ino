#include "Robo_BLE.h"

#define Robo_Name  "Robo_XYZ" // change to match the Robo name you wish to connect to
#define SCROLL_ID 3
// Requires an LED Matrix

String Text = "Arduino + Robo = <3"; // must be under 28 in total length

// Create Robo_BLE Instance
Robo_BLE RW = Robo_BLE();

void setup() {
  Serial.begin(115200);
  
  if(RW.init(Robo_Name)){
    Serial.println("Robo Connection Success!");
    Serial.println(RW.get_firmware_version());
    // Text = text to display, module_num = the matrix id, orientation 0,1,2,3 = 0,90,180,270, repeats = number of times repeated, scroll rate from 0-10, ID
    RW.scroll_text(Text, 1, 0, 1, 8, SCROLL_ID);
  }
  else Serial.println("Robo Connection Failure!");
}


void loop() {
  delay(100);
}
