/*
 * BLE + Servo Test — XIAO ESP32C3
 * Control the servo from your laptop over Bluetooth.
 * Send "L" for left, "R" for right, "C" for center.
 * Board: XIAO_ESP32C3
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>

#define SERVO_PIN D2

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_CONTROL_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_NOTIFY_UUID    "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

Servo steeringServo;
BLECharacteristic *pNotifyChar;
bool deviceConnected = false;

class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer *s) override {
    deviceConnected = true;
    Serial.println("BLE: Connected!");
  }
  void onDisconnect(BLEServer *s) override {
    deviceConnected = false;
    Serial.println("BLE: Disconnected");
    BLEDevice::startAdvertising();
  }
};

class ControlCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    String val = c->getValue().c_str();
    val.trim();
    val.toUpperCase();

    if (val == "L") {
      steeringServo.write(55);
      Serial.println("STEER LEFT");
    } else if (val == "R") {
      steeringServo.write(125);
      Serial.println("STEER RIGHT");
    } else if (val == "C") {
      steeringServo.write(90);
      Serial.println("STEER CENTER");
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== BLE Servo Test ===");

  steeringServo.attach(SERVO_PIN);
  steeringServo.write(90);
  Serial.println("Servo centered.");

  BLEDevice::init("MarioKart-Car");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCB());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pControl = pService->createCharacteristic(
    CHAR_CONTROL_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  pControl->setCallbacks(new ControlCB());

  pNotifyChar = pService->createCharacteristic(
    CHAR_NOTIFY_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pNotifyChar->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdv = BLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);
  pAdv->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising as 'MarioKart-Car'");
  Serial.println("Connect from laptop and send L/R/C");
}

void loop() {
  delay(10);
}
