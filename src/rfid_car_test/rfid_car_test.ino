/*
 * RFID Car Scanner Diagnostic v2
 * Pins: CS=D3, RST=D7, SCK=D8, MISO=D9, MOSI=D10
 */

#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS   D3
#define RFID_RST  D7

MFRC522 rfid(RFID_SS, RFID_RST);

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== RFID Car Diagnostic v2 ===");

  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_AntennaOn();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);

  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print("Version:      0x"); Serial.println(version, HEX);

  // Read TxControlReg — should be 0x83 if antenna is on
  byte txCtrl = rfid.PCD_ReadRegister(MFRC522::TxControlReg);
  Serial.print("TxControlReg: 0x"); Serial.println(txCtrl, HEX);
  Serial.println(((txCtrl & 0x03) == 0x03) ? "Antenna ON" : "Antenna OFF — forcing on...");

  // Force the antenna on and verify the write landed
  rfid.PCD_WriteRegister(MFRC522::TxControlReg, txCtrl | 0x03);
  byte txCtrl2 = rfid.PCD_ReadRegister(MFRC522::TxControlReg);
  Serial.print("TxControlReg after force write: 0x"); Serial.println(txCtrl2, HEX);

  if ((txCtrl2 & 0x03) != 0x03) {
    Serial.println("WRITE FAILED — MOSI wire may be loose. Reseat the D10 wire on both ends.");
  } else {
    Serial.println("Write OK — antenna is on. Scan a tag...");
  }
  Serial.println();
}

void loop() {
  // Re-force antenna on every loop in case something resets it
  byte txCtrl = rfid.PCD_ReadRegister(MFRC522::TxControlReg);
  if ((txCtrl & 0x03) != 0x03) {
    rfid.PCD_WriteRegister(MFRC522::TxControlReg, txCtrl | 0x03);
  }

  if (!rfid.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Card present but read failed");
    delay(200);
    return;
  }

  Serial.print("UID: ");
  for (int i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(":");
  }
  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}
