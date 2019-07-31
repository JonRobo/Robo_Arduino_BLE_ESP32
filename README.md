# Robo_Arduino_BLE_ESP32
A BLE Client library for the esp32 to interact with Robo Wunderkind<br />
This library allows for hardware hobbyists to integrate their ideas with the Robo Wunderkind product<br />
Users are encouraged to be creative in their project ideas and combine arduino sensors and electrical components into simple or complex projects with Robo Wunderkind!
<br />
<br />
<br />
<br />
Special thanks to Neil Kolban for developing a fully functional BLE Client library for the ESP32
https://github.com/nkolban/esp32-snippets


## Setup

Open the Arduino IDE and set the Board to Node32s and open the BLE_CLient Example

![BLE_CLIENT](https://user-images.githubusercontent.com/39582212/62226987-69fdae80-b3bb-11e9-9113-9960b09ceeca.png)
<br />
From this example go to Sketch -> show sketc folder or Ctrl + k to open up the source directory of BLE
<br />
The BLE Client library has a bug that needs fixing, it has been updated on github but Arduino has not included it in the ESP32 package as of this time. To fix this take the BLE folder in this repo and replace the one found in the BLE Client sketch's source directory with it.
