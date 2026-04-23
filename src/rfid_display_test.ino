/*
 * RFID + MAX7219 Display Test Sketch
 * Scans RFID tags and shows a random boost/slowdown animation on an 8x8 LED matrix.
 * Prints tag UIDs to Serial for identification.
 *
 * Board: XIAO_ESP32C3
 *
 * Wiring (shared SPI bus):
 *   RC522:   SDA=D3(GPIO5), RST=D10(GPIO10)
 *   MAX7219: CS=D2(GPIO4)
 *   Shared:  SCK=D4(GPIO6), MOSI=D6(GPIO21), MISO=D5(GPIO7)
 *
 * Libraries:
 *   - MFRC522 (by GithubCommunity)
 *   - LedControl (by Eberhard Fahle)
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LedControl.h>

// --- Pin definitions ---
#define RFID_SS   D3   // GPIO5  — RC522 SDA/CS
#define RFID_RST  D10  // GPIO10 — RC522 RST
#define MAX_CS    D2   // GPIO4  — MAX7219 CS/LOAD
#define SPI_CLK   D4   // GPIO6  — shared SCK
#define SPI_DIN   D6   // GPIO21 — shared MOSI
#define SPI_MISO  D5   // GPIO7  — shared MISO (RC522 only)

// --- Devices ---
MFRC522 rfid(RFID_SS, RFID_RST);

// LedControl does its own bit-bang SPI, but we'll manage CS manually
// so it coexists on the hardware SPI bus.
// LedControl(dataPin, clkPin, csPin, numDevices)
// We pass the same MOSI/CLK pins. LedControl bit-bangs them, which is
// fine — we just need to make sure RFID_SS is HIGH (deselected) when
// talking to the display, and MAX_CS is HIGH when talking to RFID.
LedControl lc = LedControl(SPI_DIN, SPI_CLK, MAX_CS, 1);

// --- Animation frames ---

// Up arrow (boost) — 6 frames scrolling upward
const byte ARROW_UP[8] = {
  B00011000,
  B00111100,
  B01111110,
  B11011011,
  B00011000,
  B00011000,
  B00011000,
  B00011000
};

// Down arrow (slowdown) — mirrored
const byte ARROW_DOWN[8] = {
  B00011000,
  B00011000,
  B00011000,
  B00011000,
  B11011011,
  B01111110,
  B00111100,
  B00011000
};

// Checkered flag pattern for finish
const byte CHECKERED[8] = {
  B10101010,
  B01010101,
  B10101010,
  B01010101,
  B10101010,
  B01010101,
  B10101010,
  B01010101
};

// --- Standby animation state ---
unsigned long lastStandbyUpdate = 0;
int standbyPhase = 0;          // 0..15 for a pulsing cycle
bool standbyGrowing = true;

// --- Forward declarations ---
void deselectAll();
void selectRFID();
void selectDisplay();
void showArrowAnimation(const byte arrow[8], bool scrollUp);
void showStandbyFrame();
void displayPattern(const byte pattern[8]);
void clearDisplay();

void setup() {
  // Configure CS pins as outputs and deselect both devices
  pinMode(RFID_SS, OUTPUT);
  pinMode(MAX_CS, OUTPUT);
  deselectAll();

  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== RFID + Display Test ===");
  Serial.println("Wave a tag over the RC522...");
  Serial.println();

  // Initialize hardware SPI for RFID
  SPI.begin(SPI_CLK, SPI_MISO, SPI_DIN);

  // Initialize RFID
  selectRFID();
  rfid.PCD_Init();

  // Check if RC522 is responding
  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  deselectAll();

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
  Serial.println("RC522 OK!");

  // Initialize MAX7219
  // LedControl handles its own CS, but we make sure RFID is deselected
  deselectAll();
  lc.shutdown(0, false);   // Wake up from power-saving mode
  lc.setIntensity(0, 4);   // Medium brightness (0-15)
  lc.clearDisplay(0);

  Serial.println("MAX7219 OK!");
  Serial.println();
  Serial.println("Entering standby — waiting for RFID tag...");
  Serial.println();

  // Seed the random number generator from an unconnected analog pin
  randomSeed(analogRead(A0) ^ micros());
}

void loop() {
  // --- Try to read an RFID tag ---
  deselectAll();
  selectRFID();

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // --- Tag detected! ---

    // Print UID in hex
    Serial.print("Tag UID: ");
    for (int i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
      Serial.print(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) Serial.print(":");
    }
    Serial.println();

    // Print as C byte array for copy-paste
    Serial.print("  -> byte TAG_UID[");
    Serial.print(rfid.uid.size);
    Serial.print("] = {");
    for (int i = 0; i < rfid.uid.size; i++) {
      Serial.print("0x");
      if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
      Serial.print(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) Serial.print(", ");
    }
    Serial.println("};");

    // Halt the tag
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    deselectAll();

    // --- Pick random effect: boost or slowdown ---
    bool isBoost = random(2) == 0;  // 50/50

    if (isBoost) {
      Serial.println("  >> BOOST! (up arrow)");
      showArrowAnimation(ARROW_UP, true);
    } else {
      Serial.println("  >> SLOWDOWN! (down arrow)");
      showArrowAnimation(ARROW_DOWN, false);
    }
    Serial.println();

    // Brief pause before returning to standby
    delay(500);
    clearDisplay();
    return;
  }

  deselectAll();

  // --- Standby: pulsing dot animation ---
  unsigned long now = millis();
  if (now - lastStandbyUpdate >= 80) {
    lastStandbyUpdate = now;
    showStandbyFrame();
  }

  delay(20);
}

// ============================================================
//  SPI bus management — only one device selected at a time
// ============================================================

void deselectAll() {
  digitalWrite(RFID_SS, HIGH);
  digitalWrite(MAX_CS, HIGH);
}

void selectRFID() {
  digitalWrite(MAX_CS, HIGH);
  digitalWrite(RFID_SS, LOW);
}

void selectDisplay() {
  digitalWrite(RFID_SS, HIGH);
  // MAX_CS is handled by LedControl internally
}

// ============================================================
//  Display helpers
// ============================================================

void displayPattern(const byte pattern[8]) {
  deselectAll();
  // LedControl manages MAX_CS on its own via the csPin we gave it
  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, pattern[row]);
  }
}

void clearDisplay() {
  deselectAll();
  lc.clearDisplay(0);
}

// ============================================================
//  Arrow animation — scrolls the arrow in from the bottom/top
//  then flashes it a few times
// ============================================================

void showArrowAnimation(const byte arrow[8], bool scrollUp) {
  // Phase 1: Scroll the arrow onto the display (8 frames)
  for (int frame = 0; frame < 8; frame++) {
    byte buf[8] = {0};

    if (scrollUp) {
      // Arrow enters from bottom, scrolls up
      // At frame=0, only bottom row visible (row 7 shows arrow row 7)
      // At frame=7, full arrow visible
      for (int row = 0; row < 8; row++) {
        int srcRow = row + (7 - frame);
        if (srcRow >= 0 && srcRow < 8) {
          buf[row] = arrow[srcRow];
        }
      }
    } else {
      // Arrow enters from top, scrolls down
      // At frame=0, only top row visible (row 0 shows arrow row 0)
      // At frame=7, full arrow visible
      for (int row = 0; row < 8; row++) {
        int srcRow = row - (7 - frame);
        if (srcRow >= 0 && srcRow < 8) {
          buf[row] = arrow[srcRow];
        }
      }
    }

    displayPattern(buf);
    delay(60);
  }

  // Phase 2: Hold full arrow
  displayPattern(arrow);
  delay(400);

  // Phase 3: Flash the arrow 3 times
  for (int i = 0; i < 3; i++) {
    clearDisplay();
    delay(120);
    displayPattern(arrow);
    delay(180);
  }

  // Phase 4: Quick checkered flash then back to arrow
  displayPattern(CHECKERED);
  delay(200);
  displayPattern(arrow);
  delay(300);
}

// ============================================================
//  Standby animation — pulsing diamond/dot that grows and shrinks
// ============================================================

void showStandbyFrame() {
  byte buf[8] = {0};

  // standbyPhase goes 0..7 (grow) then 8..15 (shrink)
  int size;
  if (standbyPhase < 8) {
    size = standbyPhase;
  } else {
    size = 15 - standbyPhase;
  }

  // Draw a diamond shape centered at (3.5, 3.5) with radius = size/2
  // Using a simple approach: light pixels within Manhattan distance
  int radius = size;  // 0..7
  // Scale radius to 0..3 for the 8x8 grid
  int r = radius / 2;

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      // Manhattan distance from center (3.5, 3.5) approximated
      int dx = abs(col - 3);
      int dy = abs(row - 3);
      // For the smallest size, just the center 2x2
      if (r == 0) {
        if (dx <= 0 && dy <= 0) {
          buf[row] |= (1 << (7 - col));
        }
      } else {
        if (dx + dy <= r) {
          buf[row] |= (1 << (7 - col));
        }
      }
    }
  }

  displayPattern(buf);

  // Advance phase
  standbyPhase++;
  if (standbyPhase >= 16) {
    standbyPhase = 0;
  }
}
