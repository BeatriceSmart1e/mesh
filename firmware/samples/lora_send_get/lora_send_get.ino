#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// CHANGE THIS LINE FOR EACH BOARD:
// true for master board, false for slave board
#define IS_MASTER true 

// hardware pins
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_CS    18
#define LORA_RST   23
#define LORA_DIO0  26

// oled pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA     21
#define OLED_SCL     22
#define OLED_RST     -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

int msgCounter = 0;
bool waitingForReply = false;
unsigned long lastSendTime = 0;
const unsigned long timeoutPeriod = 7000; // 7 seconds timeout if a packet drops

void updateScreen(String title, String mainMsg, String subMsg) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(title);
  display.println("");
  display.setTextSize(2);
  display.println(mainMsg);
  display.setTextSize(1);
  display.println("");
  display.print(subMsg);
  display.display();
}

void sendLoRaMessage() {
  String textToSend = IS_MASTER ? "Ping " : "Pong ";
  textToSend += msgCounter;

  Serial.print("Sending: ");
  Serial.println(textToSend);
  updateScreen(IS_MASTER ? "--- MASTER ---" : "--- SLAVE ---", "SENDING...", textToSend);

  LoRa.beginPacket();
  LoRa.print(textToSend);
  LoRa.endPacket();

  msgCounter++;
  lastSendTime = millis();
  waitingForReply = true;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("OLED failed");
    while(1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Screen Awake!");
  display.display();
  delay(1000); 

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa failed!");
    updateScreen("ERROR", "LORA FAIL", "");
    while (1);
  }

  Serial.println("LoRa Transceiver Initialized.");
  
  if (IS_MASTER) {
    // Master kicks off the entire chain instantly
    delay(2000); 
    sendLoRaMessage();
  } else {
    updateScreen("--- SLAVE ---", "Listening", "Waiting for Ping...");
  }
}

void loop() {
  // Always check for incoming radio packets
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    String incomingMsg = "";
    while (LoRa.available()) {
      incomingMsg += (char)LoRa.read();
    }

    Serial.print("Received: ");
    Serial.print(incomingMsg);
    Serial.print(" | RSSI: ");
    Serial.println(LoRa.packetRssi());

    String rssiStr = "RSSI: " + String(LoRa.packetRssi()) + " dBm";
    updateScreen(IS_MASTER ? "--- MASTER RX ---" : "--- SLAVE RX ---", incomingMsg, rssiStr);

    if (IS_MASTER) {
      // Master received a reply, clear flag, wait 2 seconds, then send the next one
      waitingForReply = false;
      delay(2000); 
      sendLoRaMessage();
    } else {
      // Slave received a message, wait 1 second, then fire a reply back
      delay(1000); 
      sendLoRaMessage();
      // After responding, immediately clear flag to listen again
      waitingForReply = false; 
    }
  }

  // Anti-Stall Safeguard (Master Only)
  // If a packet gets lost in the air, the Master will timeout and force-restart the sequence
  if (IS_MASTER && waitingForReply && (millis() - lastSendTime > timeoutPeriod)) {
    Serial.println("Packet dropped or Timeout! Resending...");
    updateScreen("--- MASTER ---", "TIMEOUT!", "Retrying...");
    delay(1000);
    sendLoRaMessage();
  }
}
