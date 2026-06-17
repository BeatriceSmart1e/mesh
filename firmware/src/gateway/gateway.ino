// this script handles actually receiving the data, and retransmitting modified data, if needed.

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
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

struct LoRaPacket {
  uint8_t senderID;
  uint8_t targetID;
  uint8_t messageID;
  uint8_t hopCount;
  uint8_t maxHops;
  uint8_t lastRepeater;
  char payload[64]; // max message length in bytes
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
const uint8_t NODE_ID = 3; // should match GATEWAY_NODE_ID in gateway.py

// init of radiolib object
SX1276 radio = new Module(LORA_CS, LORA_DIO0, LORA_RST, LORA_DIO1);

// interrupt callback for received packets
volatile bool packetReceived = false;
void setFlag(void) {
  packetReceived = true;
}

void printPacketToSerial(const LoRaPacket& p, float rssi) {
  // this function is only for debugging purposes.
  // the primary communication with RPi is via raw binary data.
  Serial.println("------ LoRa RX Packet (Arduino) ------");
  Serial.print("From: "); Serial.println(p.senderID);
  Serial.print("To: "); Serial.println(p.targetID);
  Serial.print("MsgID: "); Serial.println(p.messageID);
  Serial.print("Hop: "); Serial.println(p.hopCount);
  Serial.print("MaxHop: "); Serial.println(p.maxHops);
  Serial.print("Via: "); Serial.println(p.lastRepeater);
  Serial.print("RSSI: "); Serial.println(rssi);
  Serial.print("Payload: "); Serial.println(p.payload);
  Serial.println("------------------------------------");
}

void setup() {
  Serial.begin(9600);
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    while(1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Gateway Node Init...");
  display.display();

  // init of hardware spi pins
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  // init of radiolib
  int state = radio.begin(868.0); // starting at default of 868MHz
  
  if (state == RADIOLIB_ERR_NONE) {
    int syncState = radio.setSyncWord(0x12); 
    radio.setBandwidth(125.0); // 125 kHz
    radio.setSpreadingFactor(7); // SF7
    radio.setCodingRate(5);    // CR 4/5
    radio.invertIQ(false);

    radio.setDio0Action(setFlag, RISING);
    radio.startReceive();
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Gateway Node ID: ");
    display.println(NODE_ID);
    display.println("Ready to RX/TX!");
    display.display();
  }
  else {
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
  if (packetReceived) {
    noInterrupts();
    packetReceived = false;
    interrupts();
    
    LoRaPacket incomingPacket;
    int state = radio.readData((uint8_t*)&incomingPacket, sizeof(LoRaPacket));

    if (state == RADIOLIB_ERR_NONE) {
      float rssi = radio.getRSSI();
      // forward the raw packet data to rpi via serial
      // flush to make sure data is sent
      Serial.write((uint8_t*)&incomingPacket, sizeof(LoRaPacket));
      Serial.flush();
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.println("RX LoRa Packet:");
      display.print("From: "); display.println(incomingPacket.senderID);
      display.print("To: "); display.println(incomingPacket.targetID);
      display.print("RSSI: "); display.println((int)rssi);
      display.print("Payload: "); display.println(incomingPacket.payload);
      display.display();

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH || state == RADIOLIB_ERR_PACKET_TOO_LONG) {
      // clear error state
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("RX Error: Bad data.");
      display.display();
    }
    radio.startReceive();
  }

  // handle incoming serial data from rpi to transmit
  if (Serial.available() >= sizeof(LoRaPacket)) {
    LoRaPacket outgoingPacket;
    Serial.readBytes((uint8_t*)&outgoingPacket, sizeof(LoRaPacket));

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("TX LoRa Packet:");
    display.print("To: "); display.println(outgoingPacket.targetID);
    display.print("Hop: "); display.println(outgoingPacket.hopCount);
    display.print("Payload: "); display.println(outgoingPacket.payload);
    display.display();

    int state = radio.transmit((uint8_t*)&outgoingPacket, sizeof(LoRaPacket));
    radio.startReceive();
  }
}
