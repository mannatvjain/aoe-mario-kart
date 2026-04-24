# Log

Reverse-chronological. Each session appends what changed, what's unfinished, what to pick up next.

### 2026-04-23 — Motor Driver + RFID Debugging (Andrew + Spencer)

**Accomplished:**
- Soldered XIAO ESP32C3 headers (14 pins), DRV8833 headers, 100nF noise cap across N20 motor terminals
- Wired DRV8833: VCC→3V3, GND→GND, IN1→D0, IN2→D1, J1 jumper bridged (STBY tied HIGH), OUT1/OUT2→N20 motor via alligator clips
- Flashed motor_test.ino via Arduino IDE 2; motor spinning forward/reverse confirmed
- Wired a second RC522 (Mannat's original RC522 didn't fit the CAD chassis)
- Debugged RC522 SPI for ~2 hours. Root cause: ESP32 Arduino v3.3.8 ignores SPI.begin() custom pin remapping on XIAO_ESP32C3. SPI.begin(D4,D5,D6,D3) silently uses defaults, gives 0x00 reads.
- Fix: moved RC522 wires to default SPI pins (D8=SCK, D9=MISO, D10=MOSI), RST moved to D7, called SPI.begin() with no args
- Also discovered MOSI/MISO were physically swapped — swapping them brought version from 0xFF to 0x92 (genuine chip)
- Tags reading correctly: white card = D2:A3:9C:1B, blue fob = 26:76:BB:03

**Updated files:**
- mario_kart.ino: RFID_RST D10→D7, SPI pin comments corrected, blue fob UID updated to 26:76:BB:03
- WIRING.md: RC522 pin table updated (D7 RST, D8/D9/D10 SPI), added SPI remapping warning
- Puppet.md: logged session steps 25-29
- PLAN.md: marked XIAO header soldering and DRV8833 wiring complete

**What's unfinished:**
- Servo not yet wired into the current breadboard setup (was tested separately earlier; BLE control confirmed working)
- Full mario_kart.ino game firmware not yet tested end-to-end
- LED matrix (MAX7219) still deferred — needs additional XIAO pins or second ESP32

**Car RC522 failure (end of session):**
- The RC522 mounted in the car chassis (not the Mannat one) has a bad antenna
- Version reads 0x92 (genuine), SPI works, but TxControlReg reads 0xFF (should be 0x80)
- Force-writing TxControlReg corrupts the value → antenna never turns on → no card detection
- Diagnosis: bad internal solder joint on this module, not fixable in software
- **Resolution: use the Mannat RC522 and adjust the CAD mount to fit it, or get a new module**

**Next session should:**
1. Resolve RC522 — swap to Mannat module or get a replacement, adjust CAD mount if needed
2. Wire SG90 servo (D2 = signal, battery rail = VCC, GND = ground)
3. Flash mario_kart.ino — verify BLE advertising, RFID lap detection, motor + servo response to BLE commands
4. Tune servo angle limits (SERVO_LEFT=55, SERVO_RIGHT=125 are guesses)
5. Once chassis is assembled: mount electronics, do full race loop test
