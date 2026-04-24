# Wiring Guide

## Power Architecture

```
                    ┌──────────┐
  USB-C ──charging──│  TP4056  │
                    │ charging │
                    │  module  │
                    └──┬───┬───┘
                   B+  │   │  B-
                    ┌──┴───┴──┐
                    │  LiPo   │
                    │1S 3.7V  │
                    └──┬───┬──┘
                       │   │
              ┌────────┴───┴────────┐
              │    TP4056 OUT+/OUT- │
              │    (protected rail) │
              └──┬──────┬──────┬───┘
                 │      │      │
                 ▼      ▼      ▼
            ┌────────┐ ┌──┐  ┌─────────┐
            │DRV8833 │ │SG│  │AMS1117  │
            │  VM    │ │90│  │ 3.3V    │
            │(motor) │ │  │  │  LDO    │
            └────────┘ └──┘  └──┬──────┘
                                │ 3.3V regulated
                         ┌──────┴──────┐
                         │             │
                         ▼             ▼
                      ┌──────┐    ┌────────┐
                      │ESP32 │    │ RC522  │
                      │ 3V3  │    │ RFID   │
                      └──────┘    └────────┘
```

**Key insight:** The 1S LiPo (3.0V–4.2V) powers everything. The N20 motor and SG90 servo run at battery voltage directly. The ESP32 and RFID module need regulated 3.3V (a fully charged LiPo hits 4.2V which would fry the ESP32's 3.3V logic).

## Pin Assignments (XIAO ESP32C3)

```
XIAO ESP32C3 Pinout (top view, USB-C at top):

        ┌──[USB-C]──┐
   D0   │ ●       ● │  5V
   D1   │ ●       ● │  GND
   D2   │ ●       ● │  3V3
   D3   │ ●       ● │  D10
   D4   │ ●       ● │  D9
   D5   │ ●       ● │  D8
   D6   │ ●       ● │  D7
        └───────────┘
```

| XIAO Pin | GPIO | Connects To         | Function           |
|----------|------|---------------------|--------------------|
| D0       | 2    | DRV8833 AIN1        | Motor PWM forward  |
| D1       | 3    | DRV8833 AIN2        | Motor PWM reverse  |
| D2       | 4    | SG90 signal (orange)| Servo PWM          |
| D3       | 5    | RC522 SDA           | SPI chip select    |
| D7       | 20   | RC522 RST           | RFID reset         |
| D8       | 8    | RC522 SCK           | SPI clock (default)|
| D9       | 9    | RC522 MISO          | SPI data in (default)|
| D10      | 10   | RC522 MOSI          | SPI data out (default)|
| 3V3      | —    | LDO 3.3V output     | Regulated power in |
| GND      | —    | Common ground        | Ground             |

**NOTE:** ESP32 Arduino v3.3.8 ignores custom pin remapping in SPI.begin(). Must use default SPI pins D8/D9/D10.

**9 of 11 GPIOs used. D4/D5/D6 are free** (could add an LED or buzzer later).

## Full Wiring Connections

### 1. TP4056 Charging Module
```
TP4056 B+   →  LiPo red wire (+)
TP4056 B-   →  LiPo black wire (-)
TP4056 OUT+ →  Power rail + (breadboard red rail)
TP4056 OUT- →  Ground rail  (breadboard blue rail)
```
USB-C plugs into the TP4056 for charging. The OUT+/OUT- provide protected battery output (overdischarge + overcurrent protection).

### 2. AMS1117-3.3V LDO Voltage Regulator
```
LDO VIN  →  Power rail + (battery voltage)
LDO GND  →  Ground rail
LDO VOUT →  ESP32 3V3 pin, RC522 3.3V pin

Capacitors (REQUIRED for stability):
  10µF electrolytic:  VIN  to GND  (input decoupling)
  22µF electrolytic:  VOUT to GND  (output decoupling)
  100nF ceramic:      VOUT to GND  (high-freq noise filtering)
```

### 3. DRV8833 Motor Driver
```
DRV8833 VM    →  Power rail + (battery voltage, 3.7V)
DRV8833 GND   →  Ground rail
DRV8833 AIN1  →  XIAO D0 (GPIO 2)
DRV8833 AIN2  →  XIAO D1 (GPIO 3)
DRV8833 AOUT1 →  N20 motor terminal 1
DRV8833 AOUT2 →  N20 motor terminal 2
DRV8833 STBY  →  Power rail + (tie HIGH to enable)

Capacitor (REQUIRED - motor noise):
  100µF electrolytic:  VM to GND (close to DRV8833)
  100nF ceramic:       across N20 motor terminals (solder directly on motor)
```

### 4. SG90 Servo Motor
```
SG90 Red (VCC)    →  Power rail + (battery voltage)
SG90 Brown (GND)  →  Ground rail
SG90 Orange (SIG) →  XIAO D2 (GPIO 4)
```
Note: SG90 is rated 4.8–6V but works at 3.7V with slightly reduced torque. Fine for a lightweight rack-and-pinion.

### 5. RC522 RFID Module
```
RC522 3.3V →  LDO 3.3V output (NOT battery voltage — RC522 is 3.3V only!)
RC522 GND  →  Ground rail
RC522 RST  →  XIAO D7  (GPIO 20)
RC522 SDA  →  XIAO D3  (GPIO 5)
RC522 MOSI →  XIAO D10 (GPIO 10)  ← default MOSI
RC522 MISO →  XIAO D9  (GPIO 9)   ← default MISO
RC522 SCK  →  XIAO D8  (GPIO 8)   ← default SCK
RC522 IRQ  →  not connected (unused)
```
IMPORTANT: Use default hardware SPI pins (D8/D9/D10). ESP32 Arduino v3.3.8 ignores SPI.begin() with custom pins on XIAO_ESP32C3 — it silently falls back to defaults, causing 0x00 reads. Always call SPI.begin() with no arguments.

### 6. N20 Motor
```
Terminal 1 →  DRV8833 AOUT1
Terminal 2 →  DRV8833 AOUT2
```
Solder a 100nF ceramic capacitor directly across the motor terminals to suppress electrical noise.

## Component Checklist

### You Have
- [x] XIAO ESP32C3 (x3 — Seeed Studio 3-pack)
- [x] RC522 RFID module + 4 RFID tags
- [x] N20 motor (double shaft)
- [x] SG90 servo motor
- [x] Breadboard
- [x] Jumper wires (dupont + breadboard)

### You Need to Buy
| Component | Spec | Why | ~Price |
|-----------|------|-----|--------|
| LiPo Battery 1S | 3.7V, 500–1000mAh, JST connector | Powers everything | $5–8 |
| DRV8833 | Dual H-bridge module (breakout board) | Motor driver, handles N20 at 3.7V | $2–4 |
| AMS1117-3.3V | 3.3V LDO regulator (SOT-223 or breakout) | Steps 3.7V→3.3V for ESP32+RFID | $1–2 |
| TP4056 | USB-C variant with protection (DW01A chip) | Charges LiPo safely | $2–3 |
| 10µF electrolytic cap | 10V+ rating | LDO input decoupling | <$1 |
| 22µF electrolytic cap | 10V+ rating | LDO output decoupling | <$1 |
| 100µF electrolytic cap | 10V+ rating | Motor driver decoupling | <$1 |
| 100nF ceramic caps (x2) | 0.1µF, 50V | LDO + motor noise filtering | <$1 |

**No resistors needed** — all modules have onboard pull-ups/pull-downs. The TP4056 comes with a 1.2kΩ RPROG resistor pre-soldered (sets ~1A charge current, fine for 500mAh+ LiPo).

## Assembly Order

1. **Set up breadboard power rails** — run the TP4056 OUT+/OUT- to the red/blue rails
2. **Install the LDO** — wire VIN from power rail, add the 3 capacitors, verify 3.3V output with multimeter before connecting anything
3. **Wire ESP32** — seat it on the breadboard, connect 3V3 to LDO output, GND to ground rail
4. **Wire DRV8833** — VM to power rail, GND to ground rail, AIN1/AIN2 to GPIO 25/26, STBY to power rail, add 100µF cap
5. **Connect N20 motor** — to AOUT1/AOUT2, solder 100nF cap across terminals
6. **Wire servo** — VCC to power rail, GND to ground rail, signal to GPIO 13
7. **Wire RC522** — 3.3V from LDO output, GND to ground rail, SPI pins per table above
8. **Test each subsystem independently** before combining

## Common Mistakes to Avoid
- **DO NOT** power the RC522 from the battery rail (max 3.6V, will fry at 4.2V)
- **DO NOT** power the ESP32 3V3 pin from the battery rail (same reason)
- **DO NOT** forget the motor decoupling cap — N20 motors generate massive EMI spikes that will crash/reset the ESP32
- **DO** make sure all grounds are connected together (common ground)
- **DO** test the LDO output voltage before connecting ESP32/RFID
