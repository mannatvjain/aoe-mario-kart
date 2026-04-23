# Soldering List

Everything that needs soldering, in one session. Do all of these before reassembling on the breadboard.

## 1. XIAO ESP32C3 — Solder Header Pins

**What:** Solder the loose header pin strips onto the XIAO board so it can seat in a breadboard.

**How:**
- Take 2x 7-pin header strips (came in the XIAO box)
- Insert the **short side** of the pins through the XIAO's edge holes (from the bottom/back of the board)
- The **long side** of the pins should stick out the bottom (these go into the breadboard)
- Solder each pin from the top side of the board
- Do this for **both sides** (14 pins total)

**Tip:** Push the pins into a breadboard first, then place the XIAO on top — the breadboard holds the pins straight while you solder.

## 2. N20 Motor — Solder Noise Suppression Capacitor

**What:** Solder a 100nF ceramic capacitor directly across the N20 motor terminals.

**How:**
- Take one 100nF ceramic cap (tiny disc, marked "104")
- Solder one leg to each motor terminal
- Keep the leads short — long leads reduce effectiveness

**Why:** The N20 motor generates electrical noise spikes that will crash/reset the ESP32. This cap absorbs them.

## 3. (Optional) RC522 — Verify Pins

The RC522 already has header pins soldered. No action needed unless they're loose — in that case, reflow the solder joints.

## 4. (Optional) MAX7219 — Verify Pins

The MAX7219 already has header pins soldered on the input side. No action needed.

---

## Parts Needed at Soldering Station
- Soldering iron + solder
- XIAO ESP32C3 (the one without pins — we have 2 spares)
- 2x 7-pin header strips (from XIAO box)
- N20 motor
- 1x 100nF ceramic capacitor (marked "104")
- A breadboard (to hold pins straight while soldering)

## Time Estimate
~15 minutes total if you've soldered before. 30 if first time.
