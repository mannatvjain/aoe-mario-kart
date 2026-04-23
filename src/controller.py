"""
Mario Kart BLE Controller — Laptop Side
Connects to the ESP32 car over BLE and sends keyboard commands.

Install: pip install bleak pynput

Usage: python controller.py

Controls:
  W / Up Arrow    → Forward
  S / Down Arrow  → Reverse
  A / Left Arrow  → Steer left
  D / Right Arrow → Steer right
  Space           → Stop
  Enter           → Start race
  Q               → Quit
"""

import asyncio
import sys
from bleak import BleakClient, BleakScanner
from pynput import keyboard

# BLE UUIDs — must match the ESP32 code
SERVICE_UUID      = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHAR_CONTROL_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
CHAR_NOTIFY_UUID  = "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

DEVICE_NAME = "MarioKart-Car"

# Global state
client = None
loop = None
steering_held = False


def notification_handler(sender, data):
    """Handle notifications from the car."""
    msg = data.decode("utf-8", errors="replace")

    if msg.startswith("POWERUP:BOOST"):
        print("\n⚡ SPEED BOOST! ⚡")
    elif msg.startswith("POWERUP:SLOW"):
        print("\n🐢 SLOWDOWN! 🐢")
    elif msg.startswith("POWERUP:END"):
        print("   (powerup ended)")
    elif msg.startswith("LAP:"):
        print(f"\n🏁 {msg}")
    elif msg.startswith("FINISH"):
        parts = msg.split("|")
        print("\n" + "=" * 40)
        print("  RACE COMPLETE!")
        print("=" * 40)
        for part in parts[1:]:
            key, val = part.split(":")
            if key == "TOTAL":
                print(f"  Total time: {val}s")
            else:
                print(f"  {key}: {val}s")
        print("=" * 40)
    elif msg == "RACE_START":
        print("\n🏎️  GO GO GO! 🏎️")
    elif msg == "READY":
        print("\nCar is ready. Press ENTER to start a new race.")
    else:
        print(f"[car] {msg}")


async def send_command(cmd):
    """Send a single-char command to the car."""
    global client
    if client and client.is_connected:
        await client.write_gatt_char(CHAR_CONTROL_UUID, cmd.encode(), response=False)


def on_key_press(key):
    """Handle key press events."""
    global loop, steering_held

    cmd = None
    try:
        if key.char == 'w':
            cmd = "F"
        elif key.char == 's':
            cmd = "B"
        elif key.char == 'a':
            cmd = "L"
            steering_held = True
        elif key.char == 'd':
            cmd = "R"
            steering_held = True
        elif key.char == 'q':
            print("\nQuitting...")
            asyncio.run_coroutine_threadsafe(send_command("S"), loop)
            sys.exit(0)
    except AttributeError:
        # Special keys
        if key == keyboard.Key.up:
            cmd = "F"
        elif key == keyboard.Key.down:
            cmd = "B"
        elif key == keyboard.Key.left:
            cmd = "L"
            steering_held = True
        elif key == keyboard.Key.right:
            cmd = "R"
            steering_held = True
        elif key == keyboard.Key.space:
            cmd = "S"
        elif key == keyboard.Key.enter:
            cmd = "START"

    if cmd and loop:
        asyncio.run_coroutine_threadsafe(send_command(cmd), loop)


def on_key_release(key):
    """Center steering when left/right keys are released."""
    global loop, steering_held

    try:
        if key.char in ('a', 'd'):
            steering_held = False
            if loop:
                asyncio.run_coroutine_threadsafe(send_command("C"), loop)
    except AttributeError:
        if key in (keyboard.Key.left, keyboard.Key.right):
            steering_held = False
            if loop:
                asyncio.run_coroutine_threadsafe(send_command("C"), loop)


async def main():
    global client, loop
    loop = asyncio.get_event_loop()

    print(f"Scanning for '{DEVICE_NAME}'...")
    device = await BleakScanner.find_device_by_name(DEVICE_NAME, timeout=10)

    if not device:
        print(f"Could not find '{DEVICE_NAME}'. Make sure the car is powered on.")
        return

    print(f"Found {device.name} ({device.address}). Connecting...")

    async with BleakClient(device) as c:
        client = c
        print("Connected!")
        print()
        print("Controls:")
        print("  W/↑ = Forward   S/↓ = Reverse")
        print("  A/← = Left      D/→ = Right")
        print("  Space = Stop    Enter = Start Race")
        print("  Q = Quit")
        print()
        print("Press ENTER to start the race.")

        # Subscribe to notifications
        await c.start_notify(CHAR_NOTIFY_UUID, notification_handler)

        # Start keyboard listener in background
        listener = keyboard.Listener(on_press=on_key_press, on_release=on_key_release)
        listener.start()

        # Keep alive
        try:
            while c.is_connected:
                await asyncio.sleep(0.1)
        except KeyboardInterrupt:
            pass

        print("Disconnected.")


if __name__ == "__main__":
    asyncio.run(main())
