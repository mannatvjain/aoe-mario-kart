# Puppet Log

## Session: 2026-04-22 — RFID Setup & Tag Scanning

### Phase 1: Unboxing & Inventory

| # | Instruction | Status | Compliance | Notes |
|---|-------------|--------|------------|-------|
| 1 | Open bags: take out 1x XIAO ESP32C3 from green box, take out RC522 from anti-static bag. Check if RC522 bag contains RFID tags. | DONE | DEVIATED — took out all 3 XIAOs instead of 1 | Has: 3x XIAO, 1x RC522, 1x white card, 1x blue fob, 2x IPEX antennas, dupont wires. Only 2 RFID tags, not 4. |

| 2 | Put away 2 spare XIAOs. Keep on table: 1 XIAO, RC522, 2 tags, jumper wires, breadboard. | DONE | MATCH | Cleaned up nicely, all parts visible |
| 3 | Seat XIAO on breadboard, USB-C facing outward, straddling center channel. | DONE | DEVIATED — XIAO is on the table next to the breadboard, not seated in it | XIAO is not pushed into the breadboard yet |
| 4 | Actually push XIAO pins into the breadboard. | SKIPPED | — | Pins not soldered. Going with Option B: female dupont wires directly onto XIAO pin holes. Will solder later. |

| 5 | Separate 7 dupont wires in specific colors. | DONE | PARTIAL — picked male-to-male first, then found female-to-male ribbon cable | Ribbon is F-to-M: female end → XIAO, male end → RC522 (if RC522 has pins) |

| 6 | Check RC522 for soldered pins. | DONE | MATCH | RC522 has pins soldered. Labels L→R: SDA, SCK, MOSI, MISO, IRQ, GND, RST, 3.3V |

### Phase 2: Wiring RC522 to XIAO

| # | Instruction | Status | Compliance | Notes |
|---|-------------|--------|------------|-------|
| 7 | Connect 3.3V and GND first, send photo to verify before continuing. | DONE | DEVIATED — connected ALL 8 pins at once including IRQ, skipped verification step | User went ahead and wired everything. IRQ should not be connected. Need to verify XIAO-side connections. |
| 8 | Remove IRQ wire, show XIAO side. | DONE | — | User showed RC522 side again. Instructed to redo all wiring with specific color-to-pin mapping for traceability. |
| 9 | Wire all 7 connections to XIAO per color map. | DONE | MATCH — all 7 wires verified correct via verbal confirmation | Blue=GND, Purple=3V3, Yellow=D10, Black=D3, Grey=D4, White=D5, Aqua=D6. White moved from IRQ to MISO on RC522 side. |
| 10 | Plug XIAO into laptop via USB-C. | DONE | MATCH | Red LED on XIAO — powered and connected. Used phone charger C-to-C cable. |
| 11 | Verify RC522 LED is on + IRQ is empty. | PENDING | — | Photo 19 shows RC522 LED on. Need to confirm MISO vs IRQ wire position. |
### Phase 2b: Wiring MAX7219 LED Matrix

| # | Instruction | Status | Compliance | Notes |
|---|-------------|--------|------------|-------|
| 12 | Take MAX7219 out of bag, show input side pins. | DONE | MATCH | 5 pins: VCC, GND, DIN, CS, CLK (bottom to top). Male pins, needs F-F wires. |
| 13 | Wire MAX7219 to XIAO. | BLOCKED | — | Can't piggyback multiple wires on unsolderd XIAO pin holes. Deferred to post-solder session. |

### Phase 3: Flash & Scan Tags

| # | Instruction | Status | Compliance | Notes |
|---|-------------|--------|------------|-------|
| 13 | Flash rfid_test.ino to XIAO. | DONE | N/A (automated) | Compiled and uploaded successfully via arduino-cli. Board detected at /dev/cu.usbmodem1101. |
| 14 | Verify RC522 communication via serial. | FAILED then FIXED | — | First attempt failed — default SPI pins wrong for XIAO ESP32C3. Fixed by explicit SPI.begin(D4,D5,D6,D3). Second flash: RC522 firmware 0x92 detected. |
| 15 | Scan white RFID card. | DONE | MATCH | UID: D2:A3:9C:1B — read multiple times successfully |
| 16 | Scan blue RFID keychain fob. | DONE | MATCH | UID: 56:B3:6D:AF — read successfully |
| 17 | Save tag UIDs into game code. | DONE | N/A (automated) | White=D2:A3:9C:1B (lap), Blue=56:B3:6D:AF (powerup). Saved in mario_kart.ino. |

---

## Photos

| # | File | Description |
|---|------|-------------|
| 1 | [01-parts-table-overview.jpeg](assets/photos/01-parts-table-overview.jpeg) | Initial parts spread on table |
| 2 | [02-parts-floor-layout.jpg](assets/photos/02-parts-floor-layout.jpg) | Second parts photo — floor layout with Arduino Uno |
| 3 | [03-xiao-box-label.jpg](assets/photos/03-xiao-box-label.jpg) | XIAO ESP32C3 box label — confirms 3-pack |
| 4 | [04-xiao-box-front.jpg](assets/photos/04-xiao-box-front.jpg) | XIAO ESP32C3 box front |
| 5 | [05-unboxed-parts.jpg](assets/photos/05-unboxed-parts.jpg) | Unboxed: 3 XIAOs, RC522, 2 RFID tags, antennas |
| 6 | [06-xiao-on-breadboard.jpg](assets/photos/06-xiao-on-breadboard.jpg) | Parts laid out, XIAO next to breadboard (not seated yet) |
| 7 | [07-xiao-closeup-pins.jpg](assets/photos/07-xiao-closeup-pins.jpg) | XIAO bottom view — header pins are loose, not soldered |
| 8 | [08-dupont-wires-selected.jpg](assets/photos/08-dupont-wires-selected.jpg) | 7 wires selected — green, orange, white, yellow, blue, black, red (male-to-male, wrong type) |
| 9 | [09-dupont-ribbon-ends.jpg](assets/photos/09-dupont-ribbon-ends.jpg) | Rainbow ribbon cable — female-to-male, this is what we'll use |
| 10 | [10-rc522-closeup.jpg](assets/photos/10-rc522-closeup.jpg) | RC522 with pins soldered, labels visible: SDA SCK MOSI MISO IRQ GND RST 3.3V |
| 11 | [11-first-wires-connected.jpg](assets/photos/11-first-wires-connected.jpg) | RC522 side — all 8 pins connected (should be 7, IRQ shouldn't be connected) |
| 12 | [12-xiao-side-wiring.jpg](assets/photos/12-xiao-side-wiring.jpg) | RC522 side again — 7 wires visible, need XIAO side still |
| 13 | [13-xiao-pin-holes.jpg](assets/photos/13-xiao-pin-holes.jpg) | XIAO bottom view — confirmed XIAO-ESP32-C3, gold pin holes visible on both edges |
| 14 | [14-xiao-wiring-attempt.jpg](assets/photos/14-xiao-wiring-attempt.jpg) | XIAO with wires plugged in — angle too steep to verify pin positions |
| 15 | [15-xiao-wiring-front.jpg](assets/photos/15-xiao-wiring-front.jpg) | XIAO front view with wires — pin labels partially visible |
| 16 | [16-xiao-wiring-angle2.jpg](assets/photos/16-xiao-wiring-angle2.jpg) | XIAO wiring side angle |
| 17 | [17-xiao-wiring-angle3.jpg](assets/photos/17-xiao-wiring-angle3.jpg) | Both boards visible — RC522 in background, XIAO in foreground |
| 18 | [18-xiao-wiring-angle4.jpg](assets/photos/18-xiao-wiring-angle4.jpg) | Wide shot — full setup with breadboard, both boards, wire mess |
| 19 | [19-xiao-powered-on.jpg](assets/photos/19-xiao-powered-on.jpg) | RC522 LED on, wiring visible, need to verify MISO/IRQ |
| 20 | [20-full-setup-powered.jpg](assets/photos/20-full-setup-powered.jpg) | Full powered setup |
| 21 | [21-unknown-board.jpg](assets/photos/21-unknown-board.jpg) | MAX7219 8x8 LED dot matrix in anti-static bag |
| 22 | [22-max7219-input-pins.jpg](assets/photos/22-max7219-input-pins.jpg) | MAX7219 out of bag, 5 input pins on green header visible |
| 23 | [23-max7219-labels.jpg](assets/photos/23-max7219-labels.jpg) | MAX7219 pin labels closeup: VCC GND DIN CS CLK |
