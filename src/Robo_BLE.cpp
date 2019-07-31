/*Author Jonathan Morley Robo Technologies GmbH
 * ESP32 Robo Wunderkind BLE Client Library based on Kolban's BLEClient library 
 * https://github.com/nkolban/esp32-snippets/tree/master/cpp_utils
 * 
 * 
 */

#include "BLEDevice.h"
#include "Arduino.h"
#include "Robo_BLE.h"

static BLEUUID robo_serviceUUID("aaaaaaaa-77f1-415f-9c9e-8a22a7f02242");
static BLEUUID read_charUUID("aa000000-77f1-415f-9c9e-8a22a7f02242");
static BLEUUID read_flag_charUUID("aa000003-77f1-415f-9c9e-8a22a7f02242");
static BLEUUID write_charUUID("aa000002-77f1-415f-9c9e-8a22a7f02242");
static BLEUUID write_flag_charUUID("aa000001-77f1-415f-9c9e-8a22a7f02242");

bool Robo_Found = 0;
bool Robo_Connected = 0;
int scanTime = 5; //In seconds
BLEScan* pBLEScan;
static BLERemoteCharacteristic* pRobo_Read_Characteristic;
static BLERemoteCharacteristic* pRobo_Write_Characteristic;
static BLERemoteCharacteristic* pRobo_Read_Flag_Characteristic;
static BLERemoteCharacteristic* pRobo_Write_Flag_Characteristic;
static BLERemoteService* pRoboService;
static BLEAdvertisedDevice* myRobo;
BLEClient*  pRoboClient  = BLEDevice::createClient();

String BLEName = "";
String Last_Received_Message = "";
String chars[36] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","0","1","2","3","4","5","6","7","8","9"};

int8_t BATT_LVL    = -1;
int8_t BATT_State  = -1;
String FW_Version = "";

long Light_State = -1;
long Distance_State = -1;
long Volume_State = -1;
int8_t Button_State = -1;
int8_t Motion_State = -1;

long LineTracker_StateL = -1;
long LineTracker_StateR = -1;
long LineTracker_StateC = -1;
int8_t LineTracker_BinStateL = -1;
int8_t LineTracker_BinStateR = -1;
int8_t LineTracker_BinStateC = -1;

int8_t ACC_Pick_Up = -1;
int8_t ACC_Put_Down = -1;
int8_t ACC_Motion = -1;

uint8_t NEW_ID = 0;
uint8_t ACTION_TRIGGER_ID[100];
bool Write_OK = 0;

uint8_t image_frame[8] = {0,0,0,0,0,0,0,0};


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String Device_Name = "";
      Device_Name = advertisedDevice.getName().c_str();
      if(Device_Name.length() > 0){
         Device_Name.remove(0,3);
         if(Device_Name == BLEName){
          myRobo = new BLEAdvertisedDevice(advertisedDevice);
          Robo_Found = 1;
         }
      }
    }
};

static void read_notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData, size_t length, bool isNotify) {
    switch(pData[0]){
      case BUTTON_READ:
        Button_State = pData[2];
        break;
      case PIR_READ:
        Motion_State = pData[2];
        break;
      case DISTANCE_READ:
        Distance_State = (pData[2] + pData[3]*256);
        break;
      case VOLUME_READ:
        Volume_State = (pData[2] + pData[3]*256);
        break;
      case LIGHT_READ:
        Light_State = (pData[2] + pData[3]*256);
        break;
      case LINETRACKER_READ:
        LineTracker_StateL = (pData[2] + pData[3]*256);
        LineTracker_StateR = (pData[4] + pData[5]*256);
        LineTracker_StateC = (pData[6] + pData[7]*256);
        LineTracker_BinStateL = pData[9];
        LineTracker_BinStateR = pData[10];
        LineTracker_BinStateC = pData[11];
        break;
      case ACCELEROMETER_READ:
        ACC_Pick_Up = pData[2];
        ACC_Put_Down = pData[3];
        ACC_Motion = pData[4];
        break;
      case BATT_STATE:
        BATT_LVL = pData[2];
        BATT_State = pData[3];
        break;
      case FIRMWARE_VER:
        for(int i = 2; i<length; i++){
          FW_Version += char(pData[i]);
        }
        break;
      case ACTION_OR_TRIGGER:
        NEW_ID = pData[2];
        ACTION_TRIGGER_ID[NEW_ID] = NEW_ID;
        //Serial.println(NEW_ID);
        break;
    }
}

static void readflag_notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData, size_t length, bool isNotify) {
    // Write is confirmed by Robo
    Write_OK = 1;
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("Connected!");
    Robo_Connected = true;
  }

  void onDisconnect(BLEClient* pclient) {
    Robo_Connected = false;
    Serial.println("Disconnected!");
  }
};

void robo_scan(){
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}

bool connect_Robo() {
    Serial.print("Forming a connection to ");
    Serial.println(BLEName);
    
    pRoboClient->setClientCallbacks(new MyClientCallback());
    
    delay(100);
    
    //Connect to the remote BLE Server.
    pRoboClient->connect(myRobo);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    pRoboService = pRoboClient->getService(robo_serviceUUID);
    if (pRoboService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(robo_serviceUUID.toString().c_str());
      pRoboClient->disconnect();
      return 0;
    }
    delay(100);

    // Obtain a reference to the READ characteristic in the service of the remote BLE server.
    pRobo_Read_Characteristic = pRoboService->getCharacteristic(read_charUUID);
    if (pRobo_Read_Characteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(read_charUUID.toString().c_str());
      pRoboClient->disconnect();
      return 0;
    }
    delay(100);

    // Obtain a reference to the READ FLAG characteristic in the service of the remote BLE server.
    pRobo_Read_Flag_Characteristic = pRoboService->getCharacteristic(read_flag_charUUID);
    if (pRobo_Read_Flag_Characteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(read_flag_charUUID.toString().c_str());
      pRoboClient->disconnect();
      return 0;
    }

    delay(100);

    // Obtain a reference to the WRITE characteristic in the service of the remote BLE server.
    pRobo_Write_Characteristic = pRoboService->getCharacteristic(write_charUUID);
    if (pRobo_Write_Characteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(write_charUUID.toString().c_str());
      pRoboClient->disconnect();
      return 0;
    }
    delay(100);

    // Obtain a reference to the WRITE FLAG characteristic in the service of the remote BLE server.
    pRobo_Write_Flag_Characteristic = pRoboService->getCharacteristic(write_flag_charUUID);
    if (pRobo_Write_Flag_Characteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(write_flag_charUUID.toString().c_str());
      pRoboClient->disconnect();
      return 0;
    }
    delay(100);

    if(pRobo_Read_Characteristic->canNotify())
      pRobo_Read_Characteristic->registerForNotify(read_notifyCallback);

    if(pRobo_Read_Flag_Characteristic->canNotify())
      pRobo_Read_Flag_Characteristic->registerForNotify(readflag_notifyCallback);

      return 1;
}
void disconnect_Robo(){
  pRoboClient->disconnect();
}

//==================================================================ROBO_BLE=================================================================================================================

Robo_BLE::Robo_BLE() {
  // Create Object Instance
}

bool Robo_BLE::init(String Name){
  BLEName = Name;
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  Serial.println("Scanning....");
  robo_scan();
  if(Robo_Found){
    Serial.println("Robo Found!");
    this->BLE_CONNECTED = connect_Robo();
    delay(500); // delay after connection to make sure read/writes are ok
  }
  else return 0;

  return 1;
}

//=====================================================================SYSTEM==============================================================================================================

void Robo_BLE::play_sound(uint8_t clipNum){
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  uint8_t len = 4;
  uint8_t command[len] = {0x03,PLAY_SOUND_CLIP,0x01,clipNum};
  Write_OK = 0;
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
}

String Robo_BLE::get_firmware_version(){
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return "";
  }
  String version = "";
  int counter = 0;
  uint8_t len = 4;
  uint8_t command[len] = {0x03,FIRMWARE_VER,0x01,0x00};
  Write_OK = 0;
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  while(FW_Version == "" && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
    version = FW_Version;
    FW_Version = "";
    return version;
  }
  return "";
}

void Robo_BLE::stop_all(){
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  
  uint8_t len = 4;
  uint8_t command[len] = {0x03,STOP_ALL,0x01,0x00};
  Write_OK = 0;
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
}

void Robo_BLE::get_battery_status(){
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  uint8_t len = 4;
  uint8_t command[len] = {0x03,BATT_STATE,0x01,0x00};
  Write_OK = 0;
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
}

int8_t Robo_BLE::get_battery_level(){
  int counter = 0;
  int level = 0;
  this->get_battery_status();
  while(BATT_LVL == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
    level = BATT_LVL;
    BATT_LVL = -1;
    return level;
  }
  return -1;
}

int8_t Robo_BLE::get_battery_state(){
  int counter = 0;
  int state = 0;
  this->get_battery_status();
  while(BATT_State == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
    state = BATT_State;
    BATT_State = -1;
    return state;
  }
  return -1;
}

void Robo_BLE::change_name(String name){
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  uint8_t Length = name.length();
  
  uint8_t ble_len = Length+3;
  uint8_t command[ble_len];
  memset(command, 0, ble_len);
  command[0] = ble_len-1;
  command[1] = CHANGE_BLE_NAME;
  command[2] = ble_len-3;
  Write_OK = 0;

  for(int i = 0; i < Length+1; i++){
    command[i+3] = name.charAt(i);
  }
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, ble_len, true);
  }
  while(!Write_OK){
    delay(10);
  }
}

//=====================================================================RGB LED==============================================================================================================

// example => RW.RGB(255,0,0,1)
void Robo_BLE::RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t module_num){
  if(module_num < 1 || module_num > 6){
      Serial.println("RGB ID is incorrect, must be 1-6");
      return;
    }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  uint8_t len = 8;
  uint8_t command[len] = {0x07,RGB_SET,0x05,module_num-1,0x01,r,g,b};
  Write_OK = 0;
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
}

void Robo_BLE::RGB_Brightness(uint8_t r, uint8_t g, uint8_t b, uint8_t module_num, float brightness){
  float R = r*brightness;
  float G = g*brightness;
  float B = b*brightness;
  
  this->RGB((uint8_t)R,(uint8_t)G,(uint8_t)B,module_num);
}

//example => RW.RGB_Timed(125,30,0,1000,1,99);
void Robo_BLE::RGB_Timed(uint8_t R, uint8_t G, uint8_t B, uint16_t Time, uint8_t module_num, uint8_t id = DEFAULT_ID){
    if(module_num < 1 || module_num > 6){
      Serial.println("RGB ID is incorrect, must be 1-6");
      return;
    }
    if(!this->BLE_CONNECTED){
      Serial.println("Robo is Not Connected!");
      return;
    }
    uint8_t TimeH = Time/256;
    uint8_t TimeL = Time%256;
    uint8_t len = 11;
    uint8_t command[len] = {0x0a,RGB_BLINK_TIME,0x08,id,module_num-1,R,G,B,TimeH,TimeL,0x00};
    Write_OK = 0;
    ACTION_TRIGGER_ID[id] = 0;
    if(pRobo_Write_Characteristic->canWrite()){
      pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
    }
    while(!Write_OK){
      delay(10);
    }
}

// example => RW.RGB_Blink(0,255,0,3,500,1,99);
void Robo_BLE::RGB_Blink(uint8_t R, uint8_t G, uint8_t B, uint8_t blinks, uint16_t period, uint8_t module_num, uint8_t id = DEFAULT_ID){
    if(module_num < 1 || module_num > 6){
      Serial.println("RGB ID is incorrect, must be 1-6");
      return;
    }
    if(!this->BLE_CONNECTED){
      Serial.println("Robo is Not Connected!");
      return;
    }
    uint8_t periodH = period/256;
    uint8_t periodL = period%256;
    uint8_t len = 11;
    uint8_t command[len] = {0x0a,RGB_BLINK_TIME,0x08,id,module_num-1,R,G,B,periodH,periodL,blinks};
    Write_OK = 0;
    ACTION_TRIGGER_ID[id] = 0;
    if(pRobo_Write_Characteristic->canWrite()){
      pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
    }
    while(!Write_OK){
      delay(10);
    }
}

void Robo_BLE::RGB_RED(uint8_t module_num){
    this->RGB(255,0,0,module_num);
}

void Robo_BLE::RGB_GREEN(uint8_t module_num){
    this->RGB(0,255,0,module_num);
}

void Robo_BLE::RGB_BLUE(uint8_t module_num){
    this->RGB(0,0,255,module_num);
}

void Robo_BLE::RGB_YELLOW(uint8_t module_num){
    this->RGB(255,255,0,module_num);
}

void Robo_BLE::RGB_ORANGE(uint8_t module_num){
    this->RGB(255,128,0,module_num);
}

void Robo_BLE::RGB_PURPLE(uint8_t module_num){
    this->RGB(255,0,255,module_num);
}

void Robo_BLE::RGB_WHITE(uint8_t module_num){
    this->RGB(255,255,255,module_num);
}

void Robo_BLE::RGB_OFF(uint8_t module_num){
    this->RGB(0,0,0,module_num);
}

//=====================================================================MOTOR==============================================================================================================

void Robo_BLE::motor_pwm(uint8_t pwm, uint8_t module_num){
    if(module_num < 1 || module_num > 6){
      Serial.println("Motor ID is incorrect, must be 1-6");
      return;
    }
    if(!this->BLE_CONNECTED){
      Serial.println("Robo is Not Connected!");
      return;
    }
    uint8_t len = 5;
    uint8_t command[len] = {0x05,MOTOR_PWM,0x02,0x00,0x00};
    command[3] = module_num-1;
    command[4] = pwm;
    Write_OK = 0;
    if(pRobo_Write_Characteristic->canWrite()){
      pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
    }
    while(!Write_OK){
      delay(10);
    }
}
// example RW.motor_cw(20,1);
void Robo_BLE::motor_cw(uint8_t Speed, uint8_t module_num){
     if(module_num < 1 || module_num > 6){
      Serial.println("Motor ID is incorrect, must be 1-6");
      return;
    }
    if(Speed < 0 || Speed > 100){
      Serial.println("Speed must be 0-100%");
      return;
    }
    uint8_t pwm = uint8_t((Speed*127.0/100.0));
    this->motor_pwm(pwm, module_num);
}

// example RW.motor_ccw(20,1);
void Robo_BLE::motor_ccw(uint8_t Speed, uint8_t module_num){
    if(module_num < 1 || module_num > 6){
      Serial.println("Motor ID is incorrect, must be 1-6");
      return;
    }
    if(Speed < 0 || Speed > 100){
      Serial.println("Speed must be 0-100%");
      return;
    }
    uint8_t pwm = uint8_t((Speed*127.0/100.0));
    pwm = 256 - pwm;
    this->motor_pwm(pwm, module_num);
}

void Robo_BLE::motor_stop(uint8_t module_num){
     if(module_num < 1 || module_num > 6){
      Serial.println("Motor ID is incorrect, must be 1-6");
      return;
    }
    this->motor_pwm(0, module_num);
}

void Robo_BLE::set_drive(uint16_t vel, uint16_t distance, uint8_t dir,  uint8_t id = 99){
   if(!this->BLE_CONNECTED){
      Serial.println("Robo is Not Connected!");
      return;
   }
   uint8_t velH = vel/256;
   uint8_t velL = vel%256;
   uint8_t wdH =  0;
   uint8_t wdL = 89;
   uint8_t distanceH = distance/256;
   uint8_t distanceL = distance%256;
   
   uint8_t len = 12;
   uint8_t command[len] = {0x0b,MOTOR_DRIVE,0x09,id,0x03,dir,velH,velL,wdH,wdL,distanceH,distanceL};
   Write_OK = 0;
   ACTION_TRIGGER_ID[id] = 0;
   if(pRobo_Write_Characteristic->canWrite()){
     pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
   }
   while(!Write_OK){
     delay(10);
   }
}

void Robo_BLE::drive(uint8_t vel, uint16_t distance, bool dir, uint8_t id = 99){
   uint8_t directions = 1;
   if(!dir) directions = 2;
   if(vel < 0 || vel > 100){
      Serial.println("Velocity must be 0-100%");
      return;
    }
   float velf = (float)vel;
   vel = (uint16_t)((velf*MAX_VELOCITY)/100.0);
   this->set_drive(vel, distance, directions, id);
}

void Robo_BLE::drive_inf(uint8_t vel, bool dir){
   uint8_t directions = 1;
   if(!dir) directions = 2;
   if(vel < 0 || vel > 100){
      Serial.println("Velocity must be 0-100%");
      return;
    }
   float velf = (float)vel;
   vel = (uint16_t)((velf*MAX_VELOCITY)/100.0);
   this->set_drive(vel, INF, directions);
}

void Robo_BLE::turn(uint8_t vel, uint16_t angle, bool dir, uint8_t id = 99){
    uint8_t directions = 0;
   if(!dir) directions = 3;
   if(vel < 0 || vel > 100){
      Serial.println("Velocity must be 0-100%");
      return;
    }
   uint16_t distance = (uint16_t)((angle/360.00)*((TURNING_RAD*2*3.14159)));
   float velf = (float)vel;  
   vel = (uint16_t)((velf*MAX_VELOCITY)/100.0);
   this->set_drive(vel, distance, directions, id);
}

void Robo_BLE::turn_inf(uint8_t vel, bool dir){
   uint8_t directions = 0;
   if(!dir) directions = 3;
   if(vel < 0 || vel > 100){
      Serial.println("Velocity must be 0-100%");
      return;
    }
   float velf = (float)vel;
   vel = (uint16_t)((velf*MAX_VELOCITY)/100.0);
   this->set_drive(vel, INF, directions);
}

void Robo_BLE::stop(){
    this->motor_stop(1);
    this->motor_stop(2);
}

//=============================================================================MATRIX========================================================================================================

// example RW.scroll_text("Robo Here!", 1, 0, 1, 10, 18);  
void Robo_BLE::scroll_text(String text, uint8_t module_num, uint8_t orientation, uint8_t repeats, uint8_t scroll_rate, uint8_t id = DEFAULT_ID ){
  uint8_t Length = text.length();
  if(module_num < 1 || module_num > 8){
    Serial.println("module_num must be 1-8");
    return;
  }
  if(Length > 28){
    Serial.println("Please keep the text to 28 chars or less");
    return;
  }
  if(scroll_rate > 10 || scroll_rate < 0){
    Serial.println("Scrolling rate from 0-10");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  uint8_t ble_len;
  // case where we need to send two messages
  if(Length>11){
    ble_len = 20;
    uint8_t command[ble_len];
    memset(command, 0, ble_len);
    command[0] = ble_len-1;
    command[1] = MATRIX_SCROLL1;
    command[2] = ble_len-3;
    command[3] = Matrix_ActionIDs[module_num-1];
    command[4] = module_num-1;
    command[5] = orientation;
    command[6] = repeats;
    command[7] = scroll_rate;
    command[8] = Length;
    Write_OK = 0;
    ACTION_TRIGGER_ID[id] = 0;
    for(int i = 0; i < 20; i++){
      command[i+9] = text.charAt(i);
    }
    if(pRobo_Write_Characteristic->canWrite()){
      pRobo_Write_Characteristic->writeValue((uint8_t*)command, ble_len, true);
    }
    while(!Write_OK){
      delay(10);
    }
    memset(command, 0, ble_len);
    ble_len = 3 + Length-11;
    command[0] = ble_len-1;
    command[1] = MATRIX_SCROLL2;
    command[2] = ble_len-3;
    for(int i = 11; i < Length+1; i++){
      command[i-8] = text.charAt(i);
    }
    if(pRobo_Write_Characteristic->canWrite()){
      pRobo_Write_Characteristic->writeValue((uint8_t*)command, ble_len, true);
    }
    while(!Write_OK){
      delay(10);
    }
  }
  else{ // case where we only need to send one message
      ble_len = Length+9;
      uint8_t command[ble_len];
      memset(command, 0, ble_len);
      command[0] = ble_len-1;
      command[1] = MATRIX_SCROLL1;
      command[2] = ble_len-3;
      command[3] = Matrix_ActionIDs[module_num-1];
      command[4] = module_num-1;
      command[5] = orientation;
      command[6] = repeats;
      command[7] = scroll_rate;
      command[8] = Length;
      Write_OK = 0;
    
    for(int i = 0; i < Length+1; i++){
      command[i+9] = text.charAt(i);
    }
    if(pRobo_Write_Characteristic->canWrite()){
      pRobo_Write_Characteristic->writeValue((uint8_t*)command, ble_len, true);
    }
    while(!Write_OK){
      delay(10);
    }
  }
}

void rotate90(){
  uint8_t transposed[8] = {0,0,0,0,0,0,0,0};
  for(uint8_t i = 0; i < 8; i++){
    for(uint8_t j = 0; j < 8; j++){
      // check if there is a 1 in position 7-i
      //if yes then add 1 << j
      if((image_frame[j] & (1 << (7-i))) == (1 << (7-i))){
        transposed[i] += (1 << j);
      }
    }
  }
  memcpy(image_frame, transposed, sizeof(image_frame));
}

void Robo_BLE::matrix_image(uint8_t image[], uint8_t orientation, uint8_t module_num){
  if(module_num < 1 || module_num > 8){
    Serial.println("module_num must be 1-8");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  memcpy(image_frame, image, sizeof(image_frame));
  for(uint8_t i = orientation; i > 0; i--){
    rotate90();
  }
  
  uint8_t len = 12;
  uint8_t command[len] = {0x0b,MATRIX_IMAGE,0x09,image_frame[0],image_frame[1],image_frame[2],image_frame[3],image_frame[4],image_frame[5],image_frame[6],image_frame[7],module_num-1};
  Write_OK = 0;
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
}

//======================================================================BUTTON==============================================================================================================
// example RW.get_button_state(1);
uint8_t Robo_BLE::get_button_state(uint8_t module_num){

  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return 0;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return 0;
  }

  uint8_t len = 4;
  uint8_t command[len] = {0x03, BUTTON_READ, 0x01, module_num-1};
  uint8_t but = 0;
  int counter = 0;
  Write_OK = 0;

  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  while(Button_State == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
    but = Button_State;
    Button_State = -1;
    return but;
  }
  return 0;
}

// example RW.set_button_trigger(1,1,BUTTON_ID);
void Robo_BLE::set_button_trigger(uint8_t condition, uint8_t module_num, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  if(condition < -1){
    Serial.println("Conditions are: ");
    Serial.println("-1 Button Release Trigger ");
    Serial.println(" 0 Button Pressed Trigger ");
    Serial.println("1+ Button Clicked N Times ");
    return;
  }
  uint8_t len = 6;
  uint8_t command[len] = {0x05, BUTTON_TRIG, 0x03, id, module_num-1, condition};
  uint8_t response = 0;
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}

//========================================================================LIGHT SENSOR=======================================================================================================
// example light_state = RW.get_light_state(1);
long Robo_BLE::get_light_state(uint8_t module_num){
  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return 0;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return 0;
  }

  uint8_t len = 4;
  uint8_t command[len] = {0x03, LIGHT_READ, 0x01, module_num-1};
  long light = 0;
  int counter = 0;
  Write_OK = 0;
  
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  while(Light_State == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
    light = Light_State;
    Light_State = -1;
    return light;
  }
  return 0;
}

void Robo_BLE::set_light_trigger(uint16_t value, uint8_t comparitor, uint8_t module_num, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }

  if(comparitor < 0 || comparitor > 1){
    Serial.println("Conditions are: ");
    Serial.println("0 Less Than ");
    Serial.println("1 Greater Than ");
    return;
  }
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  uint8_t value_l = value % 256;
  uint8_t value_h = value/256;
  uint8_t len = 8;
  uint8_t command[len] = {0x07, LIGHT_TRIG, 0x05, id, module_num-1, comparitor, value_l, value_h};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  
  return;
}


//===================================================================PIR SENSOR==============================================================================================================
uint8_t Robo_BLE::get_motion_state(uint8_t module_num){
  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return 2;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return 2;
  }
  Write_OK = 0;
  int mot = 0;
  int counter = 0;
  uint8_t len = 4;
  uint8_t command[len] = {0x03, PIR_READ, 0x01, module_num-1};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  while(Motion_State == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
    mot = Motion_State;
    Motion_State = -1;
    return mot;
  }
  return 0;
}

void Robo_BLE::set_motion_trigger(bool condition, uint8_t module_num, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  uint8_t len = 6;
  uint8_t command[len] = {0x05, MOTION_TRIG, 0x03, id, module_num-1, condition};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}

//=================================================================ULTRASONIC===============================================================================================================
int Robo_BLE::get_distance_state(uint8_t module_num){
  if(module_num < 1 || module_num > 4){
    Serial.println("module_num must be 1-4");
    return -1;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return -1;
  }
  Write_OK = 0;
  int dist = 0;
  int counter = 0;
  uint8_t len = 6;
  uint8_t command[len] = {0x03, DISTANCE_READ, 0x01, module_num-1};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  while(Distance_State == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
    dist = Distance_State;
    Distance_State = -1;
    return dist;
  }
  return -1;
}

int Robo_BLE::get_sound_state(uint8_t module_num){
  if(module_num < 1 || module_num > 4){
    Serial.println("module_num must be 1-4");
    return -1;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return -1;
  }
  Write_OK = 0;
  int vol = 0;
  int counter = 0;
  uint8_t len = 4;
  uint8_t command[len] = {0x03, VOLUME_READ, 0x01, module_num-1};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  while(Volume_State == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     vol = Volume_State;
     Volume_State = -1;
     return vol;
  }
  return -1;
}

void Robo_BLE::set_distance_trigger(uint8_t value, uint8_t comparitor, uint8_t module_num, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }

   if(comparitor < 0 || comparitor > 1){
    Serial.println("Conditions are: ");
    Serial.println("0 Less Than ");
    Serial.println("1 Greater Than ");
    return;
  }
   if(value < 0 || value > 150){
    Serial.println("Trigger Value must be 0-150cm");
    return;
  }
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  uint8_t len = 7;
  uint8_t command[len] = {0x06, DISTANCE_TRIG, 0x04, id, module_num-1, comparitor, value};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}

void Robo_BLE::set_sound_trigger(uint8_t value, uint8_t comparitor, uint8_t module_num, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 6){
    Serial.println("module_num must be 1-6");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  if(comparitor < 0 || comparitor > 1){
    Serial.println("Conditions are: ");
    Serial.println("0 Less Than ");
    Serial.println("1 Greater Than ");
    return;
  }
  if(value < 0 || value > 100){
    Serial.println("Trigger Value must be 0-100% loudness");
    return;
  }
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  uint8_t len = 7;
  uint8_t command[len] = {0x06, SOUND_TRIG, 0x04, id, module_num-1, comparitor, value};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}


//=============================================================================LINE TRACKER=================================================================================================

void Robo_BLE::get_linetracker_state(uint8_t module_num){
  if(module_num < 1 || module_num > 4){
    Serial.println("module_num must be 1-4");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  Write_OK = 0;
  int counter = 0;
  uint8_t len = 4;
  uint8_t command[len] = {0x03, LINETRACKER_READ, 0x01, module_num-1};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}
// example int L = RW.get_linetracker_state_l(1);
long Robo_BLE::get_linetracker_state_l(uint8_t module_num){
  long lt_l = 0;
  int counter = 0;
  this->get_linetracker_state(module_num);
  while(LineTracker_StateL == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     lt_l = LineTracker_StateL;
     LineTracker_StateL = -1;
     return lt_l;
  }
  return -1;
}

long Robo_BLE::get_linetracker_state_c(uint8_t module_num){
  long lt_c = 0;
  int counter = 0;
  this->get_linetracker_state(module_num);
  while(LineTracker_StateC == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     lt_c = LineTracker_StateC;
     LineTracker_StateC = -1;
     return lt_c;
  }
  return -1;
}

long Robo_BLE::get_linetracker_state_r(uint8_t module_num){
  long lt_r = 0;
  int counter = 0;
  this->get_linetracker_state(module_num);
  while(LineTracker_StateR == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     lt_r = LineTracker_StateR;
     LineTracker_StateR = -1;
     return lt_r;
  }
  return -1;
}

// example int Lb = RW.get_linetracker_presence_l(1);
int8_t Robo_BLE::get_linetracker_presence_l(uint8_t module_num){
  int8_t bin_l = 0;
  int counter = 0;
  this->get_linetracker_state(module_num);
  while(LineTracker_BinStateL == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     bin_l = LineTracker_BinStateL;
     LineTracker_BinStateL = -1;
     return bin_l;
  }
  return -1;
}

int8_t Robo_BLE::get_linetracker_presence_c(uint8_t module_num){
  int8_t bin_c = 0;
  int counter = 0;
  this->get_linetracker_state(module_num);
  while(LineTracker_BinStateC == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     bin_c = LineTracker_BinStateC;
     LineTracker_BinStateC = -1;
     return bin_c;
  }
  return -1;
}

int8_t Robo_BLE::get_linetracker_presence_r(uint8_t module_num){
  int8_t bin_r = 0;
  int counter = 0;
  this->get_linetracker_state(module_num);
  while(LineTracker_BinStateR == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     bin_r = LineTracker_BinStateR;
     LineTracker_BinStateR = -1;
     return bin_r;
  }
  return -1;
}

void Robo_BLE::set_linetracker_trigger(bool condition, uint8_t module_num, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 4){
    Serial.println("module_num must be 1-4");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  uint8_t trigger_condition = !condition;
  uint8_t len = 6;
  uint8_t command[len] = {0x05, LINETRACKER_TRIG, 0x03, id, module_num-1, trigger_condition};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}

void Robo_BLE::begin_linetracking(uint8_t module_num, uint8_t motors_bitmask, uint8_t directions_bitmask, uint8_t speed, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 4){
    Serial.println("module_num must be 1-4");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  uint8_t len = 12;
  uint8_t command[len] = {0x0b, LINETRACKER_TRIG, 0x09, id, module_num-1, motors_bitmask, directions_bitmask, speed, 0x00, 0x00, 0x00, 0x00};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}

//=============================================================================ACCELEROMETER=================================================================================================

void Robo_BLE::get_accelerometer_state(uint8_t module_num){
  if(module_num < 1 || module_num > 4){
    Serial.println("module_num must be 1-4");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  Write_OK = 0;
  int counter = 0;
  uint8_t len = 4;
  uint8_t command[len] = {0x03, ACCELEROMETER_READ, 0x01, module_num-1};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}

int8_t Robo_BLE::get_acc_pickup_status(uint8_t module_num){
  int8_t pickup = -1;
  int counter = 0;
  this->get_accelerometer_state(module_num);
  while(ACC_Pick_Up == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     pickup = ACC_Pick_Up;
     ACC_Pick_Up = -1;
     return pickup;
  }
  return -1;
}

int8_t Robo_BLE::get_acc_putdown_status(uint8_t module_num){
  int8_t putdown = -1;
  int counter = 0;
  this->get_accelerometer_state(module_num);
  while(ACC_Put_Down == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     putdown = ACC_Pick_Up;
     ACC_Put_Down = -1;
     return putdown;
  }
  return -1;
}

int8_t Robo_BLE::get_acc_motion_status(uint8_t module_num){
  int8_t motion = -1;
  int counter = 0;
  this->get_accelerometer_state(module_num);
  while(ACC_Motion == -1 && counter < 1000){
    delay(1);   // wait for system response
    counter ++;
  }
  if(counter < 1000){
     motion = ACC_Motion;
     ACC_Motion = -1;
     return motion;
  }
  return -1;
}

void Robo_BLE::set_accelerometer_trigger(uint8_t condition, uint8_t module_num, uint8_t id = DEFAULT_ID){
  if(module_num < 1 || module_num > 4){
    Serial.println("module_num must be 1-4");
    return;
  }
  if(!this->BLE_CONNECTED){
    Serial.println("Robo is Not Connected!");
    return;
  }
  if(condition < 0 || condition > 2){
    Serial.println("Conditions are: ");
    Serial.println("0 Put Down\n");
    Serial.println("1 Pick Up\n");
    Serial.println("2 Motion\n");
    return;
  }
  Write_OK = 0;
  ACTION_TRIGGER_ID[id] = 0;
  uint8_t len = 6;
  uint8_t command[len] = {0x05, ACCELEROMETER_TRIG, 0x03, id, module_num-1, condition};
  if(pRobo_Write_Characteristic->canWrite()){
    pRobo_Write_Characteristic->writeValue((uint8_t*)command, len, true);
  }
  while(!Write_OK){
    delay(10);
  }
  return;
}


//===========================================================================================================================================================================================

// Use this in a loop to check if our action or trigger has happened
bool Robo_BLE::monitor_id(uint8_t id){
  if(ACTION_TRIGGER_ID[id] == id){
    ACTION_TRIGGER_ID[id] = 0; // clear this index
    return 1;
  }
  return 0;
}
