# mesh
A work-in-progress mesh network, used to help with communication and location tracking, without the need of an internet connection, let alone electricity.

**Note:** So far, this project is configured specifically for the **TTGO LoRa32 v1.6.1 by LILYGO**. It is not guaranteed to work for other boards, as it has not been tested on other boards as of right now.

**Note 2:** You will (eventually) notice a file for the "repeater" nodes, which are written slightly differently to the files I wrote for the TTGO LoRa32 modules. This is because at this moment, at home, the only extra LoRa modules were not already attached to an ESP module, so I used generic ESP32 dev boards with LoRa modules attached, and used as repeaters. In future, I will modify this to work with the intended boards.

> When using Arduino IDE to flash files, be sure the following setings are set exactly as shown:
> <img width="379" height="161" alt="Screenshot 2026-06-09 at 22 20 58" src="https://github.com/user-attachments/assets/42bf90d3-cc4f-4d33-abf4-384b5b4d4909" />
