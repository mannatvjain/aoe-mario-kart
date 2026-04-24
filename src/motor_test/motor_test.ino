// Simple motor test — spins N20 via DRV8833, no BLE needed
// Forward 2s → stop 1s → reverse 2s → stop 1s → repeat

#define MOTOR_AIN1 D0
#define MOTOR_AIN2 D1

void setup() {
  pinMode(MOTOR_AIN1, OUTPUT);
  pinMode(MOTOR_AIN2, OUTPUT);
}

void loop() {
  // Forward
  digitalWrite(MOTOR_AIN1, HIGH);
  digitalWrite(MOTOR_AIN2, LOW);
  delay(2000);

  // Stop
  digitalWrite(MOTOR_AIN1, LOW);
  digitalWrite(MOTOR_AIN2, LOW);
  delay(1000);

  // Reverse
  digitalWrite(MOTOR_AIN1, LOW);
  digitalWrite(MOTOR_AIN2, HIGH);
  delay(2000);

  // Stop
  digitalWrite(MOTOR_AIN1, LOW);
  digitalWrite(MOTOR_AIN2, LOW);
  delay(1000);
}
