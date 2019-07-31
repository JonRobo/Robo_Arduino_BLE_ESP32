#include "Robo_BLE.h"

#define Robo_Name  "Robo_XYZ" // change to match the Robo name you wish to connect to
#define DRIVE_ID 1 // a unique number to identify an executing action
#define TURN_ID 2

// Requires two motors

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

// turn(50,90,1,TURN_ID) 50 = speed in % , 90 = degrees of turn , 1 = direction CW vs CCW , TURN_ID is a unique identifier so we know when the action is done
// drive(50,75,1,DRIVE_ID) 50 = speed in % , 75 = distance to drive in cm , 1 is the direction forward or backward, DRIVE_ID is a unique identifier so we know when the action is done

void drive_demo(){
  RW.turn(50,90,1,TURN_ID);
  while(!RW.monitor_id(TURN_ID)){ // wait for the turning action to complete
    delay(100);
  }
  delay(1000);
  RW.drive(50,75,1,DRIVE_ID);
  while(!RW.monitor_id(DRIVE_ID)){  // wait for the drive action to complete
    delay(100);
  }
  delay(1000);
}

void loop() {
  drive_demo();
  delay(100);
}
