# USB2Amiga — Implementation Plan

Version: 0.1  
Date: 2026-04-28

---

## Phases Overview

| Phase | Focus                              | Deliverable                              |
|-------|------------------------------------|------------------------------------------|
| 1     | Core infrastructure                | Builds, boots, event queues working      |
| 2     | Keyboard                           | USB keyboard → Amiga keystrokes working  |
| 3     | Mouse                              | USB mouse → Amiga mouse working          |
| 4     | Configuration                      | Config file loaded and applied           |
| 5     | Joystick / Gamepad                 | USB gamepad → Amiga joystick working     |
| 6     | PCB design                         | Gerbers ready for fabrication            |
| 7     | Integration & hardening            | Tested on real hardware, release-ready   |

---

## Phase 1 — Core Infrastructure

**Goal:** Clean dual-core loop with working event queues and a verified build.

### Tasks

1. **Fix existing bugs in `usb2amiga.c`**
   - `ami_kbd_init` is called twice (once with `&p_mouse_events`) — fix to call `ami_mouse_init(&p_mouse_events)`.
   - Verify all init functions are called correctly.

2. **Implement `event_queue.c/h`**
   - Single-producer / single-consumer lock-free ring buffer, capacity 32.
   - Three instances: `kbd_queue`, `mouse_queue`, `joystick_queue`.
   - API: `queue_push()` (non-blocking, returns false if full), `queue_pop()` (non-blocking, returns false if empty).

3. **Wire event queues into core loops**
   - Core 0 input tasks push to queues.
   - Core 1 output tasks pop from queues.

4. **Debug UART**
   - Reassign UART to GPIO 20/21 (away from keyboard GPIO 0/1).
   - Confirm `print_greeting` and mount/unmount callbacks print correctly.

5. **Verify build and boot on Pico hardware**

---

## Phase 2 — Keyboard

**Goal:** USB keystrokes appear on the Amiga as correct native keystrokes.

### Tasks

1. **Implement `keymap.c/h`**
   - Full USB HID → Amiga keycode lookup table (all standard keys).
   - Function: `uint8_t usb_to_amiga_keycode(uint8_t usb_code, bool *found)`.

2. **Complete `usb_hid_kbd.c`**
   - Replace `putchar` debug output with `queue_push(&kbd_queue, event)`.
   - Handle key-down and key-up events correctly.
   - Handle modifier keys (translate to Amiga modifier keycodes).

3. **Implement `ami_kbd.c`**
   - Power-up handshake: send sync byte 0xFF, wait for ACK.
   - Bit-bang KCLK/KDAT with correct 20 µs timing using `sleep_us()`.
   - ACK detection: poll KDAT for low pulse after last bit, timeout 143 ms.
   - `ami_kbd_out_task()`: pop from `kbd_queue`, transmit keycode byte.
   - Reset warning: detect Ctrl+L.Amiga+R.Amiga combination in the event stream and send keycode 0x78.

4. **GPIO setup**
   - KCLK: GPIO 0, open-drain (drive low or set to input/high-Z).
   - KDAT: GPIO 1, open-drain.
   - /RESET: GPIO 2, input with pull-up.

5. **Test on hardware**
   - Verify with logic analyser or oscilloscope before connecting to Amiga.
   - Test on Amiga: basic typing, modifier keys, F-keys, cursor keys.

---

## Phase 3 — Mouse

**Goal:** USB mouse movement and buttons work correctly on Amiga mouse port.

### Tasks

1. **Write `pio_quadrature.pio`**
   - PIO state machine accepts signed count values from TX FIFO.
   - Outputs two-phase quadrature on two GPIO pins per axis.
   - One state machine per axis (2 total: H and V).

2. **Implement `ami_mouse.c`**
   - Initialise PIO state machines for H and V axes.
   - `ami_mouse_out_task()`: pop from `mouse_queue`, feed dx/dy to PIO FIFOs.
   - Clamp pulse rate to configured maximum (default 5500 counts/sec).
   - Drive button GPIOs directly (active low).

3. **Complete `usb_hid_mouse.c`**
   - Replace ANSI cursor debug output with `queue_push(&mouse_queue, event)`.
   - Accumulate dx/dy between reports if needed.
   - Map left/right/middle buttons and wheel to event struct.

4. **GPIO setup**
   - H-A: GPIO 6, H-B: GPIO 8 (PIO-controlled).
   - V-A: GPIO 7, V-B: GPIO 9 (PIO-controlled).
   - Buttons: GPIO 10 (middle), GPIO 11 (left), GPIO 12 (right).

5. **Test on hardware**
   - Verify quadrature waveform with oscilloscope.
   - Test on Amiga: pointer movement, button clicks, scroll wheel.

---

## Phase 4 — Configuration

**Goal:** Config file on Pico flash is loaded at boot and overrides defaults.

### Tasks

1. **Add FatFs and TinyUSB MSC**
   - Add `tinyusb_device` and FatFs to CMakeLists.
   - Format a FAT12 partition in the upper portion of Pico flash.
   - Implement USB MSC callbacks to expose the partition when config button held at boot.

2. **Implement `config.c/h`**
   - Parse `USB2AMIGA.CFG` using a minimal INI parser (no dynamic allocation).
   - Populate a `config_t` struct with all settings.
   - Fall back to compiled-in defaults for any missing/invalid key.

3. **Apply config to subsystems**
   - Pass `config_t` to `ami_mouse_init`, `ami_kbd_init`, `ami_joystick_init`, `keymap_init`.
   - Keymap overrides: merge config `[keymap]` entries into the default table.

4. **Config button behaviour**
   - GPIO 22 held low at boot → enter MSC mode, present flash as USB drive.
   - Normal boot → load config, proceed.

---

## Phase 5 — Joystick / Gamepad

**Goal:** USB gamepad D-pad and buttons drive Amiga joystick port correctly.

### Tasks

1. **Complete `usb_hid_gamepad.c`**
   - Uncomment and implement gamepad report processing.
   - Map D-pad and buttons to `joystick_event_t`, push to `joystick_queue`.
   - Apply analogue stick → digital conversion with configurable dead zone.

2. **Implement `ami_joystick.c`**
   - `ami_joystick_out_task()`: pop from `joystick_queue`, drive GPIO lines.
   - All outputs active-low, driven via 74AHCT125 buffer.

3. **GPIO setup**
   - UP: GPIO 13, DOWN: GPIO 14, LEFT: GPIO 15, RIGHT: GPIO 16.
   - BTN1: GPIO 18, BTN2: GPIO 17.

4. **Test on hardware**
   - Test with a USB gamepad on Amiga joystick port.
   - Verify D-pad, fire buttons, analogue stick in digital mode.

---

## Phase 6 — PCB Design

**Goal:** Fabrication-ready PCB for the ATX shield form factor.

### Tasks

1. **Choose EDA tool** — KiCad 8 (open source, good Gerber export).

2. **Schematic**
   - Pico module footprint + supporting passives.
   - Level shifters (74AHCT125) on all Amiga-facing outputs.
   - ESD arrays (PRTR5V0U2X) on all external connectors.
   - 5 V → 3.3 V LDO for Pico power.
   - Polyfuse on USB 5 V.
   - Config button + status LED.
   - All connectors: 5-pin DIN, 2× DB9, 2× DB25, 1× USB-A (+ hub footprint as DNP stretch).

3. **PCB layout**
   - ATX rear panel dimensions: 157 mm × 44.5 mm usable area.
   - Two-board stack if needed for connector depth.
   - Route high-speed USB traces with controlled impedance (90 Ω differential).
   - Keep Amiga signal traces short and away from USB.

4. **Review and DRC**
   - Run KiCad DRC, fix all errors.
   - Review with a second pair of eyes before ordering.

5. **Fabrication**
   - Order prototype run (e.g. JLCPCB or PCBWay, 5 boards).
   - Assemble and smoke-test.

---

## Phase 7 — Integration & Hardening

**Goal:** Reliable operation on real Amiga hardware; project ready for release.

### Tasks

1. **End-to-end latency measurement** — verify < 5 ms keyboard latency.
2. **Stress testing** — rapid key presses, fast mouse movement, simultaneous devices.
3. **Edge cases** — USB device hot-plug/unplug, hub connect/disconnect, power cycling.
4. **Keyboard passthrough** (stretch) — implement if time permits.
5. **USB hub circuit** (stretch) — validate hub PCB variant.
6. **Documentation**
   - Build instructions (firmware + PCB).
   - Bill of materials with Mouser/Farnell part numbers.
   - Wiring diagram for breadboard/prototype use.
   - Update README.
7. **Release**
   - Tag v1.0 on GitHub.
   - Publish Gerbers and BOM.

---

## Immediate Next Steps (start here)

1. Fix the `ami_kbd_init` / `ami_mouse_init` bug in `usb2amiga.c`.
2. Implement `event_queue.c/h`.
3. Implement `keymap.c/h` with the full default mapping table.
4. Complete `ami_kbd.c` with the full Amiga keyboard protocol.
5. Test keyboard on hardware with a logic analyser before connecting to Amiga.
