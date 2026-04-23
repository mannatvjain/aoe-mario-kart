/*
 * RFID Tag Scanner — Test Sketch
 * Scans tags and prints their UIDs to Serial Monitor.
 * Use this to identify your tags before loading the game code.
 *
 * Board: XIAO_ESP32C3
 */

#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS   D3   // GPIO5 — RC522 SDA/CS
#define RFID_RST  D10  // GPIO10 — RC522 RST

MFRC522 rfid(RFID_SS, RFID_RST);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== RFID Tag Scanner ===");
  Serial.println("Wave a tag over the RC522...");
  Serial.println();

  // Explicitly set SPI pins — XIAO ESP32C3 defaults don't match our wiring
  SPI.begin(D4, D5, D6, D3);  // SCK=D4(GPIO6), MISO=D5(GPIO7), MOSI=D6(GPIO21), SS=D3(GPIO5)
  rfid.PCD_Init();

  // Check if RC522 is responding
  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: Cannot communicate with RC522!");
    Serial.println("Check your wiring:");
    Serial.println("  RC522 SCK  -> XIAO D4");
    Serial.println("  RC522 MOSI -> XIAO D6");
    Serial.println("  RC522 MISO -> XIAO D5");
    Serial.println("  RC522 SDA  -> XIAO D3");
    Serial.println("  RC522 RST  -> XIAO D10");
    Serial.println("  RC522 3.3V -> XIAO 3V3");
    Serial.println("  RC522 GND  -> XIAO GND");
    while (1) delay(1000);
  }

  Serial.print("RC522 firmware version: 0x");
  Serial.println(version, HEX);
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

  // Print as C array for easy copy-paste into game code
  Serial.print("  -> byte TAG_UID[4] = {");
  for (int i = 0; i < rfid.uid.size; i++) {
    Serial.print("0x");
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(", ");
  }
  Serial.println("};");
  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
