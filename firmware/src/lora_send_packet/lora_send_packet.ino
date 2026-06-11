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
const uint8_t NODE_ID = 1; 
uint8_t msgCnt = 0;

struct LoRaPacket {
  uint8_t senderID;
  uint8_t targetID;
  uint8_t messageID;
  char payload[64]; // max message length in bytes
};

void setup() {
  Serial.begin(115200);
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

  Serial.println("LoRa Sender");

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

}

void loop() {
  LoRaPacket p; // packet

  p.senderID = NODE_ID; // set to 1 by default
  p.targetID = 2; // sending to node 2
  p.messageID = msgCnt++;

  strncpy(p.payload, "hello, world!", sizeof(p.payload)); // copies text to payload

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&p, sizeof(p));
  LoRa.endPacket();

  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.setTextSize(1);
  display.println("--- LORA STATUS ---");
  display.println("");
  
  display.setTextSize(2);
  display.print("Sent: #");
  display.println(msgCnt);
  display.display();

  Serial.print("Sent Packet ID: ");
  Serial.println(p.messageID);

  delay(5000);
  
}
