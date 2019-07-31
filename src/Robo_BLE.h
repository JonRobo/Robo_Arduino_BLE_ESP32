#ifndef Robo_BLE_h
#define Robo_BLE_h

#include "Arduino.h"
#include "BLEDevice.h"

#define MAX_VELOCITY 350.0
#define MAX_DISTANCE 200.0
#define TURNING_RAD 9.1
#define INF 65535
#define WHEEL_DIAMETER 89
#define DEFAULT_ID 99

#define CHANGE_BLE_NAME    0X06
#define FIRMWARE_VER       0X07
#define BATT_STATE         0x10
#define STOP_ALL           0x30

#define MOTOR_PWM          0x50
#define MATRIX_IMAGE       0x52
#define RGB_SET            0x53
#define PLAY_SOUND_CLIP    0x61
#define BUTTON_READ        0x85
#define PIR_READ           0x83
#define LIGHT_READ         0x80
#define DISTANCE_READ      0x84
#define VOLUME_READ        0x81
#define LINETRACKER_READ   0x86
#define ACCELEROMETER_READ 0x87

#define RGB_BLINK_TIME     0xA2
#define MOTOR_DRIVE        0xA6
#define MATRIX_SCROLL1     0xA7
#define MATRIX_SCROLL2     0xA8
#define SET_LINETRACKING   0xA9

#define DISTANCE_TRIG      0xB0
#define BUTTON_TRIG        0xB1
#define LIGHT_TRIG         0xB2
#define MOTION_TRIG        0xB3
#define SOUND_TRIG         0xB4
#define LINETRACKER_TRIG   0xB5
#define ACCELEROMETER_TRIG 0xB6
#define ACTION_OR_TRIGGER  0xc0

class Robo_BLE
{
  public:
    Robo_BLE();
    bool init(String Name);
    
    // System
    void play_sound(uint8_t clipNum);
    void stop_all();
    void change_name(String name);
    void get_battery_status();
    int8_t get_battery_level();
    int8_t get_battery_state();
    String get_firmware_version();
    
    // RGB
    void RGB(uint8_t R, uint8_t G, uint8_t B, uint8_t module_num);
    void RGB_OFF(uint8_t module_num);
    void RGB_RED(uint8_t module_num);
    void RGB_GREEN(uint8_t module_num);
    void RGB_BLUE(uint8_t module_num);
    void RGB_PURPLE(uint8_t module_num);
    void RGB_YELLOW(uint8_t module_num);
    void RGB_ORANGE(uint8_t module_num);
    void RGB_WHITE(uint8_t module_num);
    
    void RGB_Blink(uint8_t R, uint8_t G, uint8_t B, uint8_t blinks, uint16_t period, uint8_t module_num, uint8_t id);
    void RGB_Timed(uint8_t R, uint8_t G, uint8_t B, uint16_t Time, uint8_t module_num, uint8_t id);
    void RGB_Brightness(uint8_t r, uint8_t g, uint8_t b, uint8_t module_num, float brightness);
    
    // Motor
    void motor_pwm(uint8_t pwm, uint8_t module_num);
    void motor_cw(uint8_t Speed, uint8_t module_num);
    void motor_ccw(uint8_t Speed, uint8_t module_num);
    void motor_stop(uint8_t module_num);
  
    void set_drive(uint16_t vel, uint16_t distance, uint8_t dir, uint8_t id);
    void drive(uint8_t vel, uint16_t distance, bool dir, uint8_t id);
    void stop();
    void turn(uint8_t vel, uint16_t angle, bool dir, uint8_t id);
    void drive_inf(uint8_t vel, bool dir);
    void turn_inf(uint8_t vel, bool dir);
    
    // LED Matrix
    void scroll_text(String text, uint8_t module_num, uint8_t orientation, uint8_t repeats, uint8_t scroll_rate, uint8_t id);
    void matrix_image(uint8_t image[], uint8_t orientation, uint8_t module_num);
 
    // Button
    void set_button_trigger(uint8_t condition, uint8_t module_num, uint8_t id);
    uint8_t get_button_state(uint8_t module_num);
    
    // Light Sensor
    long get_light_state(uint8_t module_num);
    void set_light_trigger(uint16_t value, uint8_t comparitor, uint8_t module_num, uint8_t id);

    // PIR Sensor
    uint8_t get_motion_state(uint8_t module_num);
    void set_motion_trigger(bool condition, uint8_t module_num, uint8_t id);

    // Ultrasonic
    int get_distance_state(uint8_t module_num);
    void set_distance_trigger(uint8_t value, uint8_t comparitor, uint8_t module_num, uint8_t id);
    
    int get_sound_state(uint8_t module_num);
    void set_sound_trigger(uint8_t value, uint8_t comparitor, uint8_t module_num, uint8_t id);

    // Line Tracker
    long get_linetracker_state_l(uint8_t module_num);
    long get_linetracker_state_c(uint8_t module_num);
    long get_linetracker_state_r(uint8_t module_num);
    int8_t get_linetracker_presence_l(uint8_t module_num);
    int8_t get_linetracker_presence_c(uint8_t module_num);
    int8_t get_linetracker_presence_r(uint8_t module_num);
    void set_linetracker_trigger(bool condition, uint8_t module_num, uint8_t id);
    void begin_linetracking(uint8_t module_num, uint8_t motors_bitmask, uint8_t directions_bitmask, uint8_t speed, uint8_t id);
    
    // Accelerometer
    int8_t get_acc_pickup_status(uint8_t module_num);
    int8_t get_acc_putdown_status(uint8_t module_num);
    int8_t get_acc_motion_status(uint8_t module_num);
    void set_accelerometer_trigger(uint8_t condition, uint8_t module_num, uint8_t id);
    
    bool monitor_id(uint8_t id);
    bool BLE_CONNECTED = 0;

  private:

    // Line Tracker
    void get_linetracker_state(uint8_t module_num);
    
    // Accelerometer
    void get_accelerometer_state(uint8_t module_num);
  
    const uint8_t RGB_ActionIDs[6]       = {0,1,2,3,4,5};
    const uint8_t Motor_ActionIDs[6]     = {6,7,8,9,10,11};
    const uint8_t Servo_ActionIDs[6]     = {12,13,14,15,16,17};
    const uint8_t Matrix_ActionIDs[6]    = {18,19,20,21,22,23};
    const uint8_t Button_TriggerIDs[6]   = {24,25,26,27,28,29};
    const uint8_t Motion_TriggerIDs[6]   = {30,31,32,33,34,35};
    const uint8_t Light_TriggerIDs[6]    = {36,37,38,39,40,41};
    const uint8_t Distance_TriggerIDs[6] = {42,43,44,45,46,47};
    const uint8_t Sound_TriggerIDs[6]    = {48,49,50,51,52,53};
};

#endif
