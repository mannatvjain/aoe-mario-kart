# Andrew Handoff

Hey Andrew — here's where we are and what needs to happen next.

## What's Working (tested and verified)

| System | Status |
|--------|--------|
| RFID tag scanning | WORKING — RC522 reads tags, UIDs saved in code |
| BLE wireless control | WORKING — laptop connects to XIAO over Bluetooth |
| Servo steering over BLE | WORKING — A/D keys steer left/right wirelessly |
| Game firmware | WRITTEN — not yet fully tested as a unit |
| Laptop controller | WRITTEN — Python script, keyboard control |

## What's NOT Working Yet

| System | Why |
|--------|-----|
| Motor driving | DRV8833 not wired yet — have the board, just need to connect |
| Full game loop | Need motor + RFID + servo all wired to one XIAO |
| MAX7219 LED display | Blocked until XIAO headers are soldered |
| Power system (battery) | Have parts, not wired — currently running off USB |

## What Needs to Happen (in order)

### 1. Test Motor Driver (you + Mannat, ~10 min)
Wire the DRV8833 to the XIAO and N20 motor. Verify the motor spins forward/reverse from BLE commands. Can use alligator clips — no soldering needed.

**Wiring:**
```
DRV8833 VM   → XIAO 3V3 (for bench test — will be battery later)
DRV8833 GND  → XIAO GND
DRV8833 AIN1 → XIAO D0 (left side, 1st hole from top)
DRV8833 AIN2 → XIAO D1 (left side, 2nd hole from top)
DRV8833 STBY → XIAO 3V3 (tie high to enable)
DRV8833 AOUT1 → N20 motor wire 1
DRV8833 AOUT2 → N20 motor wire 2
```

### 2. Test Everything Together (~15 min)
Wire RFID + servo + motor all on one XIAO. Flash the full game code. Run the laptop controller and verify:
- Car drives forward/reverse
- Steering works
- RFID tags trigger powerups and count laps
- Timer works and reports results over BLE

### 3. Hand Off to Famin for Soldering
Famin solders headers onto XIAO + noise cap on motor. See `SOLDER_LIST.md`.

### 4. Meera Assembles Chassis
Mount all electronics on the car. RFID reader goes on the underside facing the track.

### 5. Night Session — Final Integration
Everyone meets up. Verify everything works on the actual car on the actual track.

## Key Files

| File | What |
|------|------|
| `WIRING.md` | Every wire connection, pin map, power architecture |
| `SOLDER_LIST.md` | What Famin needs to solder |
| `FAMIN_HANDOFF.md` | Famin's full handoff doc |
| `src/mario_kart.ino` | Main game firmware |
| `src/controller.py` | Laptop BLE controller |
| `Puppet.md` | Full build log with photos |

## Hardware You're Working With

- **XIAO ESP32C3** — tiny board, USB-C, 11 GPIO pins. NOT a standard ESP32 DevKit.
- **DRV8833** — small black motor driver breakout (~1cm square)
- **N20 motor** — gold cylinder with 2 wires
- **SG90 servo** — blue box, 3 wires (brown=GND, red=VCC, orange=signal)
- **RC522** — blue RFID reader board with antenna coil pattern

## Gotchas

- XIAO has NO soldered headers — use male-to-male jumper wires poked into pin holes
- `SPI.begin()` must specify pins explicitly: `SPI.begin(D4, D5, D6, D3)`
- DO NOT power RC522 from battery rail — 3.3V only
- The N20 motor WILL crash the ESP32 without a noise suppression cap (100nF across terminals) — for bench testing it's usually OK but expect occasional resets

## Team

- **Mannat** — electronics + code
- **Famin** — soldering + assembly
- **Meera** — car chassis
- **Andrew** — you're here

## Questions?

Run Claude Code in this repo — full context in CLAUDE.md, Puppet.md, and all the docs.
