#include <SPI.h>
#include <SD.h>

const int sdCsPin = 13;

void setup() {
  Serial.begin(115200);
  // this part is important as spi library is more general, not specific to our lilygo
  SPI.begin(14, 2, 15, 13);  // SCK, MISO, MOSI, CS
  
  if (!SD.begin(sdCsPin)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  Serial.println("SD Card initialized successfully.");

  // write
  File dataFile = SD.open("/datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Hello TTGO LoRa32");
    dataFile.close();
    Serial.println("Write completed.");
  } else {
    Serial.println("Error opening file for writing.");
  }

  // read
  File fileToRead = SD.open("/datalog.txt", FILE_READ);
  if (fileToRead) {
    Serial.println("Reading datalog.txt:");
    while (fileToRead.available()) {
      Serial.write(fileToRead.read());
    }
    fileToRead.close();
  } else {
    Serial.println("Error opening file for reading.");
  }
}

void loop() {
  // misc
}
