# Project Cairn: a mesh network project
A work-in-progress mesh network, used to help with communication and location tracking, without the need of an internet connection, let alone electricity.

**Note:** So far, this project is configured specifically for the **TTGO LoRa32 v1.6.1 by LILYGO**. It is not guaranteed to work for other boards, as it has not been tested on other boards as of right now.

**Note 2:** Gateway node was designed to work with RPi 4B for easy traffic routing. I decided on this as I would implement a webserver at some point to be able to use smartphones as nodes. So this is just making life easier :D

> When using Arduino IDE to flash files, be sure the following setings are set exactly as shown:
> <img width="379" height="161" alt="Screenshot 2026-06-09 at 22 20 58" src="https://github.com/user-attachments/assets/42bf90d3-cc4f-4d33-abf4-384b5b4d4909" />

> When compiling and uploading with ```arduino-cli```, ensure you install the board library for esp32, and then use the following commands to FIRST compile, THEN upload:
> ```
> arduino-cli compile  --fqbn esp32:esp32:ttgo-lora32 gateway
> ```
> ```
> arduino-cli upload -p /dev/tty[ACM/USB]X --fqbn esp32:esp32:ttgo-lora32 gateway
> ```
> [ACM/USB] means that you adjust based on your serial port, X represents the number. An example would be: ```ttyUSB0```.
