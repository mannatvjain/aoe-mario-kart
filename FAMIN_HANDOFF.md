# Famin Handoff

Hey Famin — this doc gets you up to speed on where we are and what you need to do.

## Project Summary

We're building a timed-trial Mario Kart RC car for AoE. The car drives on a track, reads RFID tags for powerups, and is controlled wirelessly from a laptop over Bluetooth.

## What's Already Done

| System | Status | Notes |
|--------|--------|-------|
| RFID tag scanning | WORKING | RC522 talks to XIAO over SPI. Both tags scanned (white card = lap marker, blue fob = powerup). |
| BLE wireless control | WORKING | Laptop connects to XIAO over Bluetooth. Commands sent and received. |
| Servo steering | WORKING | SG90 responds to BLE commands (left/right/center). Tested wirelessly from laptop. |
| Game firmware | WRITTEN | `src/mario_kart.ino` — full game code with BLE, RFID, motor, servo, lap timing, powerups. Not yet tested as a whole. |
| Laptop controller | WRITTEN | `src/controller.py` — WASD keyboard control over BLE. |
| Wiring guide | COMPLETE | `WIRING.md` — every connection documented with pin assignments for XIAO ESP32C3. |

## What You Need to Do

### Priority 1: Soldering (see SOLDER_LIST.md)

1. **Solder header pins onto a XIAO ESP32C3** — so it seats in the breadboard properly. We've been friction-fitting female wires into bare holes and it's janky.
2. **Solder 100nF cap across N20 motor terminals** — noise suppression, required or the motor will crash the ESP32.

### Priority 2: Breadboard Assembly (after soldering)

Once the XIAO has pins, seat it on the breadboard and wire everything per `WIRING.md`:

1. **Power system:** TP4056 → LiPo → breadboard power rails → AMS1117-3.3V LDO → XIAO 3V3
   - Add capacitors: 10µF on LDO input, 22µF + 100nF on LDO output, 100µF on DRV8833 VM
   - **TEST LDO OUTPUT WITH MULTIMETER BEFORE CONNECTING XIAO** (must read 3.3V)
2. **DRV8833 motor driver:** VM to power rail, AIN1→D0, AIN2→D1, AOUT1/AOUT2→N20 motor
3. **SG90 servo:** VCC to power rail, GND to ground rail, signal→D2
4. **RC522 RFID:** 3.3V from LDO (NOT battery rail), SPI pins per WIRING.md
5. **MAX7219 display:** shares SPI bus with RC522, CS→D2 (need to reassign — check with Mannat)

### Priority 3: Final Flash & Test

Once everything is wired:
```bash
# Make sure arduino-cli is installed and ESP32 board support is set up
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 src/mario_kart/mario_kart.ino
arduino-cli upload -p /dev/cu.usbmodem1101 --fqbn esp32:esp32:XIAO_ESP32C3 src/mario_kart/mario_kart.ino
```

Then on the laptop:
```bash
pip install bleak pynput
python src/controller.py
```

## Key Files

| File | What it is |
|------|-----------|
| `WIRING.md` | Complete wiring diagram, pin map, assembly order |
| `SOLDER_LIST.md` | Everything that needs soldering |
| `src/mario_kart.ino` | Main game firmware |
| `src/controller.py` | Laptop BLE controller |
| `src/rfid_test/` | Standalone RFID scanner (for testing) |
| `src/servo_test/` | Servo sweep test |
| `src/ble_servo_test/` | BLE + servo test |
| `Puppet.md` | Full build log with photos |

## Important Gotchas

- **The XIAO ESP32C3 is NOT a standard ESP32 DevKit.** It has only 11 GPIO pins. All pin assignments in the code use D0-D10 notation.
- **SPI.begin() must specify pins explicitly** — the XIAO's default SPI pins don't match our wiring. This is already handled in the code: `SPI.begin(D4, D5, D6, D3)`.
- **DO NOT power RC522 from the battery rail** — it's 3.3V only. A fully charged LiPo hits 4.2V and will fry it. Always power from the LDO output.
- **DO NOT skip the motor capacitor** — the N20 will generate EMI spikes that reset the ESP32.
- **Test the LDO output voltage with a multimeter** before connecting the XIAO. If it reads anything other than ~3.3V, something is wrong.

## Tag UIDs (already in the code)

| Tag | UID | Role |
|-----|-----|------|
| White card | `{0xD2, 0xA3, 0x9C, 0x1B}` | Lap marker |
| Blue fob | `{0x56, 0xB3, 0x6D, 0xAF}` | Powerup |
| TBD | — | Need 2 more tags |
| TBD | — | Need 2 more tags |

## Questions?

Run Claude Code in this repo — it has full context in CLAUDE.md, Puppet.md, and the code itself.
