#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h> // changed to radiolib as it will be better for the long term, it is a much better library.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// hardware pins (ttgo lora32)
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_CS 18
#define LORA_RST 23
#define LORA_DIO0 26
#define LORA_DIO1 33 // specifically for RadioLib, defined for SX1276 tracking

// oled pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1 // -1 indicates to share the same reset pin as on the system

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
const uint8_t NODE_ID = 2;

// init of radiolib object
SX1276 radio = new Module(LORA_CS, LORA_DIO0, LORA_RST, LORA_DIO1);

struct LoRaPacket {
  uint8_t senderID;
  uint8_t targetID;
  uint8_t messageID;
  char payload[64]; // max message length in bytes
};

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 OLED allocation failed");
    while(1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  Serial.println("LoRa Receiver");

  // init of hardware spi pins
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  // init of radiolib - change frequency if needs be
  Serial.print(F("[SX1276] Initializing ... "));
  int state = radio.begin(915.0); // starting at default of 915MHz
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));

    // configuring handshake
    int syncState = radio.setSyncWord(0x12); 
    if (syncState != RADIOLIB_ERR_NONE) {
        Serial.println("Failed to set sync word!");
    }
  
    radio.setBandwidth(125.0); // 125 kHz
    radio.setSpreadingFactor(7); // SF7
    radio.setCodingRate(5);    // CR 4/5
    radio.invertIQ(false);

    radio.startReceive();
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Ready to Receive!");
    display.display();
  }
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("LoRa Init Failed: ");
    display.println(state);
    display.display();
    while (1);
  }
  delay(1500);
  
}

void loop() {
  // temporary packet for parsing it
  LoRaPacket incomingPacket;

  int state = radio.readData((uint8_t*)&incomingPacket,sizeof(LoRaPacket));

  if (state == RADIOLIB_ERR_NONE) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("--- LORA STATUS ---");
    display.println("");

    if (incomingPacket.targetID == NODE_ID || incomingPacket.targetID==0) {
      Serial.println("--- NEW PACKET RECEIVED ---");
      Serial.print("From Node: "); Serial.println(incomingPacket.senderID);
      Serial.print("To Node:   "); Serial.println(incomingPacket.targetID);
      Serial.print("Msg Seq #: "); Serial.println(incomingPacket.messageID);
      Serial.print("Payload:   "); Serial.println(incomingPacket.payload);
      Serial.println("---------------------------\n");

      display.println("NEW PACKET RECEIVED!");
      display.print("From Node: "); display.println(incomingPacket.senderID);
      display.print("To Node:   "); display.println(incomingPacket.targetID);
      display.print("Msg Seq #: "); display.println(incomingPacket.messageID);
      display.print("Payload:   "); display.println(incomingPacket.payload);
      display.display();
    }
    else {
      Serial.print("Packet ignored. Meant for Node: ");
      Serial.println(incomingPacket.targetID);

      display.print("Packet ignored.\nMeant for Node: ");
      display.println(incomingPacket.targetID);
      display.display();
    }
    radio.startReceive();
  }
  else if (state==RADIOLIB_ERR_CRC_MISMATCH || state==RADIOLIB_ERR_PACKET_TOO_LONG) { // if data is corrupt/invalid
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("--- LORA STATUS ---\n");
    
    Serial.println("Error: Corrupt packet or layout size mismatch.");
    display.println("Error: Received bad data or structure mismatch.");
    display.display();
    radio.startReceive();
  }

  delay(10); // best to add delay at the end for a cooldown
  
}
