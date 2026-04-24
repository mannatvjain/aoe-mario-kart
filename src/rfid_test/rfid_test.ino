/*
 * RFID Tag Scanner — Test Sketch
 * Scans tags and prints their UIDs to Serial Monitor.
 *
 * Board: XIAO_ESP32C3
 * SPI pins: default (SCK=D8, MISO=D9, MOSI=D10)
 */

#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS   D3   // RC522 SDA/CS
#define RFID_RST  D7   // RC522 RST (moved from D10)

MFRC522 rfid(RFID_SS, RFID_RST);

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("=== RFID Tag Scanner ===");

  SPI.begin(); // use default pins: SCK=D8, MISO=D9, MOSI=D10
  rfid.PCD_Init();
  rfid.PCD_AntennaOn();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max); // max gain for clone chips

  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print("RC522 version: 0x");
  Serial.println(version, HEX);

  if (version == 0x00) {
    Serial.println("ERROR: No response from RC522. Check wiring.");
    while (1) delay(1000);
  }

  Serial.println("RC522 is working! Scan a tag...");
  Serial.println();
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  Serial.print("Tag UID: ");
  for (int i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(":");
  }
  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
