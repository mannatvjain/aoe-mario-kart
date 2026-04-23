/*
 * Servo Sweep Test — XIAO ESP32C3
 * Sweeps the SG90 servo back and forth to verify wiring.
 * Board: XIAO_ESP32C3
 */

#include <ESP32Servo.h>

#define SERVO_PIN D2  // GPIO4

Servo steeringServo;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Servo Sweep Test ===");

  steeringServo.attach(SERVO_PIN);
  steeringServo.write(90);  // Center
  delay(500);
  Serial.println("Servo centered at 90. Starting sweep...");
}

void loop() {
  // Sweep left
  Serial.println("Sweeping to 55 (left)");
  steeringServo.write(55);
  delay(1000);

  // Center
  Serial.println("Centering at 90");
  steeringServo.write(90);
  delay(1000);

  // Sweep right
  Serial.println("Sweeping to 125 (right)");
  steeringServo.write(125);
  delay(1000);

  // Center
  Serial.println("Centering at 90");
  steeringServo.write(90);
  delay(1000);
}
