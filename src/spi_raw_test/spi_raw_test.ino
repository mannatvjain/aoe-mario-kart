// GPIO + software SPI diagnostic for RC522
// Tests if MISO (D5) is stuck low, then tries bit-bang SPI

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("=== GPIO + Software SPI Diagnostic ===");

  // --- Test 1: Is D5 (MISO) stuck low? ---
  pinMode(D5, INPUT);
  Serial.print("D5 no pullup:   "); Serial.println(digitalRead(D5));

  pinMode(D5, INPUT_PULLUP);
  delay(10);
  Serial.print("D5 with pullup: "); Serial.println(digitalRead(D5));
  // 0 = D5 shorted to GND (bad wire or wrong row)
  // 1 = D5 floating or connected to RC522 MISO (expected)

  // --- Test 2: Software SPI (bit-bang) read of VersionReg ---
  // Reset RC522 first
  pinMode(D10, OUTPUT);
  digitalWrite(D10, LOW);
  delay(10);
  digitalWrite(D10, HIGH);
  delay(50);

  pinMode(D3, OUTPUT); // CS
  pinMode(D4, OUTPUT); // SCK
  pinMode(D6, OUTPUT); // MOSI
  pinMode(D5, INPUT);  // MISO

  digitalWrite(D4, LOW);  // SCK idle low (Mode 0)
  digitalWrite(D3, HIGH); // CS idle high

  byte txByte = 0xEE; // read command for VersionReg
  byte rxByte = 0;

  digitalWrite(D3, LOW); // Assert CS

  // Send address byte
  for (int i = 7; i >= 0; i--) {
    digitalWrite(D6, (txByte >> i) & 1);
    delayMicroseconds(10);
    digitalWrite(D4, HIGH);
    delayMicroseconds(10);
    digitalWrite(D4, LOW);
  }

  // Read data byte
  for (int i = 7; i >= 0; i--) {
    digitalWrite(D6, 0);
    delayMicroseconds(10);
    digitalWrite(D4, HIGH);
    delayMicroseconds(10);
    rxByte |= (digitalRead(D5) << i);
    digitalWrite(D4, LOW);
  }

  digitalWrite(D3, HIGH); // Deassert CS

  Serial.print("Software SPI VersionReg: 0x");
  Serial.println(rxByte, HEX);
  // 0x91 or 0x92 = RC522 working
  // 0x00 = MISO stuck low
  // 0xFF = MISO stuck high
}

void loop() {}
