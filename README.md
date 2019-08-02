# Robo_Arduino_BLE_ESP32
A BLE Client library for the esp32 to interact with Robo Wunderkind<br />
This library allows for hardware hobbyists to integrate their ideas with the Robo Wunderkind product<br />
Users are encouraged to be creative in their project ideas and combine arduino sensors and electrical components into simple or complex projects with Robo Wunderkind!
<br />
<br />
Special thanks to Neil Kolban for developing a fully functional BLE Client library for the ESP32
https://github.com/nkolban/esp32-snippets


## Setup

Open the Arduino IDE and set the Board to Node32s and open the BLE_CLient Example
If you have not installed the ESP32 Board to your Ardunio IDE, do so by following the instructions here
https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

![BLE_CLIENT](https://user-images.githubusercontent.com/39582212/62226987-69fdae80-b3bb-11e9-9113-9960b09ceeca.png)
<br />
<br />
From this example go to Sketch -> show sketc folder or Ctrl + k to open up the source directory of BLE
<br />
<br />
The BLE Client library has a bug that needs fixing, it has been updated on github but Arduino has not included it in the ESP32 package as of this time. To fix this take the BLE folder in the linked repo and replace the one found in the BLE Client sketch's source directory with it. https://github.com/JonRobo/BLE-Client-Fix
<br />
<br />
![replace_ble](https://user-images.githubusercontent.com/39582212/62228583-3f612500-b3be-11e9-8c1e-4c1535037b22.png)
<br />
<br />

After this is done copy paste Robo_Arduino_BLE_ESP32 into the Arduino Library folder. Close and reopen the Arduino IDE. Open the Connect example from this repository, change the Robo Name to the Robo you wish to connect to and have fun!
<br />
<br />

![first_demo](https://user-images.githubusercontent.com/39582212/62228806-b39bc880-b3be-11e9-8760-8c6f35d711c2.png)
