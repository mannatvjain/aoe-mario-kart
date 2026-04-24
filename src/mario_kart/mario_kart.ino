/*
 * AoE Mario Kart — Timed Trial with RFID Powerups
 * ESP32 + DRV8833 + SG90 + RC522 + BLE
 *
 * Install these libraries in Arduino IDE:
 *   1. ESP32Servo  (by Kevin Harrington)
 *   2. MFRC522     (by GithubCommunity)
 *   3. ArduinoBLE  is NOT used — we use the built-in ESP32 BLE stack
 *
 * Board: "XIAO_ESP32C3" in Arduino IDE
 *   - Add Seeed board URL: https://raw.githubusercontent.com/nicekwell/SeeedArduino/main/package_seeeduino_boards_index.json
 *   - Or search "esp32" in Board Manager and install Espressif's ESP32 package
 *   - Select "XIAO_ESP32C3" as board
 * Upload speed: 921600
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// ─── Pin Definitions (XIAO ESP32C3) ───────────────────────────
// XIAO ESP32C3 GPIO map:
//   D0=GPIO2, D1=GPIO3, D2=GPIO4, D3=GPIO5
//   D4=GPIO6,  D5=GPIO7,  D6=GPIO21, D7=GPIO20
//   D8=GPIO8(SCK), D9=GPIO9(MISO), D10=GPIO10(MOSI)
//
// SPI pins: use DEFAULT hardware SPI only — ESP32 Arduino v3.3.8
// ignores custom pin remapping in SPI.begin(). Call SPI.begin() with
// no args. RC522 must be wired to D8(SCK), D9(MISO), D10(MOSI).
//
#define MOTOR_AIN1  D0   // GPIO2  — DRV8833 AIN1 (PWM)
#define MOTOR_AIN2  D1   // GPIO3  — DRV8833 AIN2 (PWM)
#define SERVO_PIN   D2   // GPIO4  — SG90 signal
#define RFID_SS     D3   // GPIO5  — RC522 SDA/CS
#define RFID_RST    D7   // GPIO20 — RC522 RST
// D8(SCK), D9(MISO), D10(MOSI) used by hardware SPI for RC522

// ─── Motor PWM Config ──────────────────────────────────────────
#define PWM_FREQ       1000
#define PWM_RESOLUTION 8     // 0–255

// ─── Game Config ───────────────────────────────────────────────
#define TOTAL_LAPS        3
#define BASE_SPEED        180   // PWM duty (0–255), tune to your track
#define BOOST_SPEED       255   // Full send
#define SLOW_SPEED        100   // Nerfed
#define POWERUP_DURATION  2000  // ms
#define BOOST_CHANCE      50    // % chance of boost (vs slowdown)
#define RFID_COOLDOWN     2000  // ms between reads of same tag
#define SERVO_CENTER      90    // degrees — straight
#define SERVO_LEFT        55    // degrees — full left (tune this)
#define SERVO_RIGHT       125   // degrees — full right (tune this)

// ─── BLE UUIDs ─────────────────────────────────────────────────
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_CONTROL_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // laptop → car
#define CHAR_NOTIFY_UUID    "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"  // car → laptop

// ─── Objects ───────────────────────────────────────────────────
Servo steeringServo;
MFRC522 rfid(RFID_SS, RFID_RST);
BLECharacteristic *pControlChar;
BLECharacteristic *pNotifyChar;

// ─── Game State ────────────────────────────────────────────────
enum GameState { WAITING, RACING, FINISHED };
GameState gameState = WAITING;

int currentSpeed = 0;
int activeSpeed  = BASE_SPEED;
int servoAngle   = SERVO_CENTER;

int lapCount = 0;
unsigned long raceStartTime   = 0;
unsigned long lapStartTime    = 0;
unsigned long lapTimes[TOTAL_LAPS];

unsigned long powerupEndTime  = 0;
bool powerupActive = false;

unsigned long lastRfidReadTime = 0;
byte lastTagUID[4] = {0};

bool deviceConnected = false;

// ─── RFID Tag Assignments ──────────────────────────────────────
// Set these to your actual tag UIDs after scanning them (see Serial output)
// Tag 0 = lap marker (start/finish line)
// Tags 1-3 = powerup tags (placed around track)
byte LAP_TAG_UID[4]     = {0xD2, 0xA3, 0x9C, 0x1B};  // White card — lap marker
byte POWERUP_TAG_1[4]   = {0x26, 0x76, 0xBB, 0x03};  // Blue fob — powerup
byte POWERUP_TAG_2[4]   = {0x00, 0x00, 0x00, 0x00};  // TBD — need 2 more tags
byte POWERUP_TAG_3[4]   = {0x00, 0x00, 0x00, 0x00};  // TBD — need 2 more tags

// ─── Forward Declarations ──────────────────────────────────────
void handleCommand(String cmd);
void notify(String msg);
void startRace();
void completeLap();
void finishRace();
void triggerPowerup();

// ─── BLE Callbacks ─────────────────────────────────────────────
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    deviceConnected = true;
    Serial.println("BLE: Connected");
  }
  void onDisconnect(BLEServer *pServer) override {
    deviceConnected = false;
    Serial.println("BLE: Disconnected");
    // Restart advertising so we can reconnect
    BLEDevice::startAdvertising();
  }
};

class ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override {
    String val = pChar->getValue().c_str();
    handleCommand(val);
  }
};

// ─── Motor Control ─────────────────────────────────────────────
void motorForward(int speed) {
  ledcWrite(MOTOR_AIN1, speed);
  ledcWrite(MOTOR_AIN2, 0);
}

void motorReverse(int speed) {
  ledcWrite(MOTOR_AIN1, 0);
  ledcWrite(MOTOR_AIN2, speed);
}

void motorStop() {
  ledcWrite(MOTOR_AIN1, 0);
  ledcWrite(MOTOR_AIN2, 0);
}

// ─── Command Handler ──────────────────────────────────────────
void handleCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "START" && gameState == WAITING) {
    startRace();
    return;
  }

  if (gameState != RACING) return;

  if (cmd == "F") {
    motorForward(activeSpeed);
    currentSpeed = activeSpeed;
  } else if (cmd == "B") {
    motorReverse(activeSpeed);
    currentSpeed = -activeSpeed;
  } else if (cmd == "S") {
    motorStop();
    currentSpeed = 0;
  } else if (cmd == "L") {
    servoAngle = SERVO_LEFT;
    steeringServo.write(servoAngle);
  } else if (cmd == "R") {
    servoAngle = SERVO_RIGHT;
    steeringServo.write(servoAngle);
  } else if (cmd == "C") {
    // Center steering
    servoAngle = SERVO_CENTER;
    steeringServo.write(servoAngle);
  }
}

// ─── Race Logic ────────────────────────────────────────────────
void startRace() {
  gameState = RACING;
  lapCount = 0;
  raceStartTime = millis();
  lapStartTime = raceStartTime;
  activeSpeed = BASE_SPEED;
  powerupActive = false;

  notify("RACE_START");
  Serial.println("RACE STARTED!");
}

void completeLap() {
  unsigned long now = millis();
  lapTimes[lapCount] = now - lapStartTime;
  lapCount++;

  char msg[64];
  snprintf(msg, sizeof(msg), "LAP:%d TIME:%.2f", lapCount, lapTimes[lapCount - 1] / 1000.0);
  notify(msg);
  Serial.println(msg);

  if (lapCount >= TOTAL_LAPS) {
    finishRace();
  } else {
    lapStartTime = now;
  }
}

void finishRace() {
  gameState = FINISHED;
  motorStop();
  steeringServo.write(SERVO_CENTER);

  unsigned long totalTime = millis() - raceStartTime;

  // Build results string
  String results = "FINISH|TOTAL:";
  results += String(totalTime / 1000.0, 2);
  for (int i = 0; i < TOTAL_LAPS; i++) {
    results += "|L" + String(i + 1) + ":" + String(lapTimes[i] / 1000.0, 2);
  }
  notify(results);
  Serial.println(results);

  // After 5 seconds, return to waiting state
  delay(5000);
  gameState = WAITING;
  notify("READY");
}

// ─── Powerup Logic ─────────────────────────────────────────────
void triggerPowerup() {
  int roll = random(100);
  if (roll < BOOST_CHANCE) {
    // BOOST
    activeSpeed = BOOST_SPEED;
    notify("POWERUP:BOOST");
    Serial.println("POWERUP: BOOST!");
  } else {
    // SLOWDOWN
    activeSpeed = SLOW_SPEED;
    notify("POWERUP:SLOW");
    Serial.println("POWERUP: SLOWDOWN!");
  }
  powerupActive = true;
  powerupEndTime = millis() + POWERUP_DURATION;

  // Apply immediately if car is moving
  if (currentSpeed > 0) motorForward(activeSpeed);
  else if (currentSpeed < 0) motorReverse(activeSpeed);
}

void updatePowerup() {
  if (powerupActive && millis() > powerupEndTime) {
    activeSpeed = BASE_SPEED;
    powerupActive = false;
    notify("POWERUP:END");
    Serial.println("Powerup ended, back to base speed");

    // Apply base speed if car is moving
    if (currentSpeed > 0) motorForward(activeSpeed);
    else if (currentSpeed < 0) motorReverse(activeSpeed);
  }
}

// ─── RFID Handling ─────────────────────────────────────────────
bool uidMatch(byte *a, byte *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

bool isSameAsLastTag(byte *uid) {
  return uidMatch(uid, lastTagUID) && (millis() - lastRfidReadTime < RFID_COOLDOWN);
}

void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  byte *uid = rfid.uid.uidByte;

  // Print UID to serial (useful for initial tag identification)
  Serial.print("RFID Tag UID: ");
  for (int i = 0; i < 4; i++) {
    Serial.printf("%02X ", uid[i]);
  }
  Serial.println();

  // Cooldown check — don't re-trigger same tag immediately
  if (isSameAsLastTag(uid)) {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  // Save this tag
  memcpy(lastTagUID, uid, 4);
  lastRfidReadTime = millis();

  if (gameState != RACING) {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  // Check if it's the lap marker
  if (uidMatch(uid, LAP_TAG_UID)) {
    completeLap();
  }
  // Check if it's a powerup tag
  else if (uidMatch(uid, POWERUP_TAG_1) ||
           uidMatch(uid, POWERUP_TAG_2) ||
           uidMatch(uid, POWERUP_TAG_3)) {
    triggerPowerup();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ─── BLE Notify Helper ────────────────────────────────────────
void notify(String msg) {
  if (deviceConnected && pNotifyChar) {
    pNotifyChar->setValue(msg.c_str());
    pNotifyChar->notify();
  }
}

// ─── Setup ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("Mario Kart Timed Trial — Booting...");

  // Motor PWM setup (ESP32 Arduino v3.x pin-based API)
  ledcAttach(MOTOR_AIN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_AIN2, PWM_FREQ, PWM_RESOLUTION);
  motorStop();

  // Servo setup
  steeringServo.attach(SERVO_PIN);
  steeringServo.write(SERVO_CENTER);

  // RFID setup
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_AntennaOn();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max); // clone chips (0xFF) need max gain
  Serial.println("RFID reader initialized");

  // Seed random from analog noise
  randomSeed(analogRead(0));

  // BLE setup
  BLEDevice::init("MarioKart-Car");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Control characteristic (laptop writes commands to this)
  pControlChar = pService->createCharacteristic(
    CHAR_CONTROL_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  pControlChar->setCallbacks(new ControlCallbacks());

  // Notify characteristic (car sends events to laptop)
  pNotifyChar = pService->createCharacteristic(
    CHAR_NOTIFY_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pNotifyChar->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising as 'MarioKart-Car'");
  Serial.println("Waiting for connection...");
}

// ─── Main Loop ─────────────────────────────────────────────────
void loop() {
  if (gameState == RACING) {
    checkRFID();
    updatePowerup();
  } else if (gameState == WAITING) {
    // Still check RFID in waiting mode to print UIDs (for setup)
    checkRFID();
  }

  delay(10);  // Small delay to prevent watchdog issues
}
