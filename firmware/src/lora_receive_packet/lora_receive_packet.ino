#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// hardware pins
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_CS 18
#define LORA_RST 23
#define LORA_DIO0 26

// oled pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1 // -1 indicates to share the same reset pin as on the system

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
const uint8_t NODE_ID = 2;

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

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa Init Failed!");
    display.display();
    while (1);
  }

  Serial.println("LoRa successfully initialized!");
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Ready to Receive!");
  display.display();
  delay(1500);
  
}

void loop() {
  int p_sz = LoRa.parsePacket();

  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.setTextSize(1);
  display.println("--- LORA STATUS ---");
  display.println("");

  if (p_sz>0) {
    if (p_sz==sizeof(LoRaPacket)) {
      LoRaPacket incomingPacket;
      LoRa.readBytes((uint8_t*)&incomingPacket, sizeof(LoRaPacket));
      // we treat targetID=0 as a global ID - i.e. everyone is supposed to receive this message.
      if (incomingPacket.targetID == NODE_ID || incomingPacket.targetID == 0) {
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

        display.print("Packet ignored. Meant for Node: ");
        display.println(incomingPacket.targetID);
        display.display();
      }
    }
    else {
      Serial.println("Error: Received packet, but does not match correct struct size. Unknown sender's packet was received.");
      display.println("Error: Received packet, but does not match correct struct size. Unknown sender's packet was received.");
      display.display();
    }
  }

}
