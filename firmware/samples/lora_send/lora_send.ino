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

int counter = 0;

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

  Serial.println("LoRa successfully initialized!");
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Ready to Send!");
  display.display();
  delay(1500);
  
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.setTextSize(1);
  display.println("--- LORA STATUS ---");
  display.println("");
  
  display.setTextSize(2);
  display.print("Sent: #");
  display.println(counter);
  display.display();

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello! ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(1000);
}
