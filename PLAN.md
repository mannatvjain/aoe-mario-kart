# Plan

## Phase 1: Hardware Setup
- [x] Wire RC522 RFID module to XIAO ESP32C3
- [x] Scan and record RFID tag UIDs (white=D2:A3:9C:1B, blue=56:B3:6D:AF)
- [x] Verify RC522 firmware (0x92)
- [ ] Solder headers onto XIAO ESP32C3
- [ ] Buy remaining components (LiPo, DRV8833, AMS1117 LDO, TP4056, caps)
- [ ] Wire DRV8833 motor driver + N20 motor
- [ ] Wire SG90 servo for rack-and-pinion steering
- [ ] Wire TP4056 charging circuit + AMS1117 LDO power rail
- [ ] Wire MAX7219 LED matrix (blocked on soldering)
- [ ] Wire force sensor to bumper (collision detection)
- [ ] If pins exhausted: wire second ESP32, establish inter-ESP comms

## Phase 2: Firmware — Motor & Steering
- [ ] PWM motor control via DRV8833 (forward/reverse)
- [ ] PID speed control for consistent motor behavior
- [ ] Servo steering with configurable rotation limits (tune for rack-and-pinion)
- [ ] Game controller input: 2 joysticks (left/right steering + forward/back throttle)

## Phase 3: Firmware — Game Logic
- [ ] Internal lap timer (starts on first RFID scan)
- [ ] Lap counting via RFID (3 laps to finish)
- [ ] Record final time after 3 laps
- [ ] Force sensor bumper: configurable threshold → collision slowdown penalty
- [ ] RFID powerup system (blue fob triggers powerup effect)

## Phase 4: Communication & Leaderboard
- [ ] BLE/Wi-Fi link from ESP32 to laptop
- [ ] Send lap times + final time to laptop
- [ ] Laptop leaderboard application (track per-user scores)
- [ ] Controller input over BLE (joystick data → ESP32)

## Phase 5: Integration & Polish
- [ ] End-to-end test: controller → drive → laps → leaderboard
- [ ] Tune PID parameters
- [ ] Tune servo rotation range for steering geometry
- [ ] Tune force sensor collision threshold
- [ ] Mount electronics on car chassis
