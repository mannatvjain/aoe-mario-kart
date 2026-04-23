# AoE Mario Kart

A timed-trial Mario Kart RC car with RFID powerups, BLE wireless control, and PID motor control. Built for Columbia's Art of Engineering course.

## Key Features

The car runs a timed 3-lap race on a physical track. An RFID sensor on the underside reads tags embedded in the track surface, triggering random speed boosts or slowdowns. A laptop connects wirelessly over Bluetooth Low Energy to steer and drive the car using keyboard input.

Notable capabilities include:
- Wireless BLE control from a laptop (WASD / arrow keys)
- RFID-based powerup system with randomized boosts and slowdowns
- Servo-driven rack-and-pinion steering
- PWM motor control via DRV8833 H-bridge driver
- In-game lap timer with results reported back to the laptop over BLE
- Rechargeable LiPo power system with USB-C charging

## Hardware

| Component | Role |
|-----------|------|
| Seeed XIAO ESP32C3 | Microcontroller (WiFi + BLE) |
| RC522 RFID module | Reads track-embedded RFID tags |
| DRV8833 | H-bridge motor driver for N20 motor |
| N20 double-shaft motor | Drive motor |
| SG90 micro servo | Rack-and-pinion steering |
| MAX7219 8x8 LED matrix | Displays powerup/lap animations |
| TP4056 + 1S LiPo + AMS1117-3.3V | Power and charging system |

Full wiring diagram and pin assignments are in [`WIRING.md`](WIRING.md).

## Repository Structure

```
src/
  mario_kart.ino          # Main game firmware (BLE + RFID + motor + servo)
  controller.py           # Laptop-side BLE controller (Python)
  rfid_test/              # Standalone RFID tag scanner
  servo_test/             # Servo sweep test
  ble_servo_test/         # BLE + servo combined test
  rfid_display_test.ino   # RFID + MAX7219 display test

WIRING.md                 # Complete wiring guide with pin assignments
Puppet.md                 # Build log with step-by-step photos
```

## Setup

### Prerequisites

- [Arduino CLI](https://arduino.github.io/arduino-cli/) or Arduino IDE
- ESP32 board support (`esp32:esp32` platform)
- Python 3.8+ with `bleak` and `pynput` (for the laptop controller)

### Install Dependencies

```bash
# Arduino
arduino-cli core install esp32:esp32
arduino-cli lib install MFRC522 LedControl ESP32Servo

# Python (laptop controller)
pip install bleak pynput
```

### Flash the Car

```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 src/mario_kart/mario_kart.ino
arduino-cli upload -p /dev/cu.usbmodem1101 --fqbn esp32:esp32:XIAO_ESP32C3 src/mario_kart/mario_kart.ino
```

### Run the Controller

```bash
python src/controller.py
```

Controls: `W`/`S` = forward/reverse, `A`/`D` = steer, `Space` = stop, `Enter` = start race, `Q` = quit.

## How It Works

```
┌─────────┐   BLE    ┌──────────────┐   SPI   ┌────────┐
│ Laptop  │ ──────── │ XIAO ESP32C3 │ ──────── │ RC522  │
│ (Python)│          │              │          │ (RFID) │
└─────────┘          │   Firmware   │          └────────┘
                     │              │   PWM    ┌─────────┐
                     │              │ ──────── │ DRV8833 │ ── N20 Motor
                     │              │          └─────────┘
                     │              │   PWM    ┌─────────┐
                     │              │ ──────── │  SG90   │
                     └──────────────┘          │ (Servo) │
                                               └─────────┘
```

1. The laptop sends drive commands (forward, reverse, steer) over BLE
2. The ESP32 translates commands into PWM signals for the motor driver and servo
3. The RFID sensor continuously scans for tags on the track
4. When a tag is detected: lap marker increments the lap count, powerup tags trigger a random boost or slowdown
5. After 3 laps, the ESP32 sends the total time and lap splits back to the laptop over BLE

## Team

Built by Mannat Jain and Famin Ahmmed for Columbia University's Art of Engineering (Spring 2026).
