# USB2Amiga — Technical Specification

Version: 0.1  
Date: 2026-04-28  
Author: Joel Hammond-Turner

---

## 1. System Architecture

The RP2040's two cores are used to cleanly separate USB host input from Amiga hardware output:

```
Core 0 (Input)                     Core 1 (Output)
─────────────────────────────       ──────────────────────────────
TinyUSB host task                   ami_kbd_out_task()
usb_hid_kbd_task()    ──events──>   ami_mouse_out_task()
usb_hid_mouse_task()  ──events──>   ami_joystick_out_task()
usb_hid_gamepad_task()──events──>   led_blinking_task()
```

Events are passed between cores via three lock-free ring buffers (one per device type), sized to absorb bursts. The Pico SDK multicore FIFO is used only for signalling; the ring buffers live in shared SRAM.

---

## 2. GPIO Pin Assignment

### 2.1 Amiga Keyboard Port (5-pin DIN)

| Signal  | DIN Pin | Pico GPIO | Direction      | Notes                        |
|---------|---------|-----------|----------------|------------------------------|
| KCLK    | 2       | GPIO 0    | Output (OD)    | Open-drain, pulled up to 5V  |
| KDAT    | 3       | GPIO 1    | Output (OD)    | Open-drain, pulled up to 5V  |
| /RESET  | 5       | GPIO 2    | Input          | Pulled up; monitor for reset |
| +5V     | 1       | VSYS      | Power in       | Via diode + polyfuse         |
| GND     | 4       | GND       | —              |                              |

> KCLK and KDAT are open-drain. Use a 2N7002 or similar N-FET with 4.7 kΩ pull-up to 5 V. Pico GPIO drives the gate; drain connects to the signal line.

### 2.2 DB9 Mouse Port (Port 1)

| Signal   | DB9 Pin | Pico GPIO | Direction | Notes                    |
|----------|---------|-----------|-----------|--------------------------|
| H-PULSE  | 1       | GPIO 6    | Output    | Horizontal quadrature A  |
| V-PULSE  | 2       | GPIO 7    | Output    | Vertical quadrature A    |
| H-PULSE2 | 3       | GPIO 8    | Output    | Horizontal quadrature B  |
| V-PULSE2 | 4       | GPIO 9    | Output    | Vertical quadrature B    |
| BTN-MID  | 5       | GPIO 10   | Output    | Middle button (active low)|
| +5V      | 7       | —         | Power     | Passthrough from Amiga   |
| BTN-LEFT | 6       | GPIO 11   | Output    | Left button (active low) |
| BTN-RIGHT| 9       | GPIO 12   | Output    | Right button (active low)|
| GND      | 8       | GND       | —         |                          |

### 2.3 DB9 Joystick Port (Port 2)

| Signal | DB9 Pin | Pico GPIO | Direction | Notes           |
|--------|---------|-----------|-----------|-----------------|
| UP     | 1       | GPIO 13   | Output    | Active low      |
| DOWN   | 2       | GPIO 14   | Output    | Active low      |
| LEFT   | 3       | GPIO 15   | Output    | Active low      |
| RIGHT  | 4       | GPIO 16   | Output    | Active low      |
| BTN2   | 5       | GPIO 17   | Output    | Fire 2 (active low) |
| +5V    | 7       | —         | Power     | Passthrough     |
| BTN1   | 6       | GPIO 18   | Output    | Fire 1 (active low) |
| GND    | 8       | GND       | —         |                 |

### 2.4 Miscellaneous

| Function        | Pico GPIO | Notes                              |
|-----------------|-----------|------------------------------------|
| Config button   | GPIO 22   | Active low, internal pull-up       |
| Status LED      | GPIO 25   | On-board LED (watchdog blink)      |
| USB D+          | USB       | Native USB via Pico USB port       |
| UART TX (debug) | GPIO 0*   | Remapped to GPIO 20 for debug UART |

> *Debug UART uses GPIO 20/21 to avoid conflict with keyboard lines.

---

## 3. Amiga Keyboard Protocol

### 3.1 Protocol Summary

The Amiga keyboard protocol is a synchronous serial interface where the **keyboard drives both clock and data**. The Pico emulates the keyboard role:

- Data is transmitted MSB first, 8 bits per keycode
- Each bit: KDAT is set, then KCLK pulses low for 20 µs, then high for 20 µs
- After all 8 bits, the Amiga pulls KDAT low for ≥ 1 µs as an ACK
- The Pico must wait for ACK before sending the next keycode
- Keycode byte format: `[key6 key5 key4 key3 key2 key1 key0 updown]`
  - Bits 7–1: Amiga raw keycode (7-bit)
  - Bit 0: 0 = key down, 1 = key up

### 3.2 Timing Requirements

| Parameter         | Value      |
|-------------------|------------|
| Clock period      | 40 µs (25 kHz) |
| Clock low time    | 20 µs      |
| Clock high time   | 20 µs      |
| Inter-key gap     | ≥ 200 µs   |
| ACK timeout       | 143 ms     |
| Sync sequence     | 1 byte 0xFF on power-up |

### 3.3 Reset Sequence

When the Amiga detects Ctrl+Left Amiga+Right Amiga held simultaneously, it asserts /RESET low. The Pico monitors GPIO 2 for this. No firmware action is required beyond not interfering with the line.

The keyboard reset warning (sent by the keyboard before a reset) is transmitted as keycode 0x78 (key down) followed by 0x78 (key up).

### 3.4 Power-up Handshake

On power-up the Pico must:
1. Wait for the Amiga to release KCLK (high)
2. Send the sync byte 0xFF
3. Wait for ACK
4. Begin normal operation

---

## 4. USB HID Keycode → Amiga Keycode Mapping

### 4.1 Default Mapping (selected keys)

| USB HID Key        | USB Code | Amiga Keycode | Notes                        |
|--------------------|----------|---------------|------------------------------|
| A–Z                | 0x04–0x1D| 0x20–0x39     | Standard layout              |
| 1–0 (top row)      | 0x1E–0x27| 0x01–0x0A     |                              |
| Return             | 0x28     | 0x44          |                              |
| Escape             | 0x29     | 0x45          |                              |
| Backspace          | 0x2A     | 0x41          |                              |
| Tab                | 0x2B     | 0x42          |                              |
| Space              | 0x2C     | 0x40          |                              |
| Left Ctrl          | 0xE0     | 0x63          | Amiga Ctrl                   |
| Left Shift         | 0xE1     | 0x60          |                              |
| Left Alt           | 0xE2     | 0x64          | Amiga Left Alt               |
| Left GUI (Win/Cmd) | 0xE3     | 0x66          | Left Amiga key               |
| Right GUI          | 0xE7     | 0x67          | Right Amiga key              |
| Right Alt          | 0xE6     | 0x65          | Amiga Right Alt              |
| Right Shift        | 0xE5     | 0x61          |                              |
| Caps Lock          | 0x39     | 0x62          |                              |
| F1–F10             | 0x3A–0x43| 0x50–0x59     |                              |
| F11                | 0x44     | 0x4B          | Mapped to `(` on Amiga numpad|
| F12                | 0x45     | 0x4C          | Mapped to `)` on Amiga numpad|
| Print Screen       | 0x46     | 0x5F          | Help key                     |
| Insert             | 0x49     | 0x66*         | Configurable                 |
| Delete             | 0x4C     | 0x46          | Amiga Del                    |
| Home               | 0x4A     | 0x6F*         | Configurable                 |
| End                | 0x4D     | 0x6F*         | Configurable                 |
| Page Up            | 0x4B     | 0x6C*         | Configurable                 |
| Page Down          | 0x4E     | 0x6D*         | Configurable                 |
| Cursor keys        | 0x4F–0x52| 0x4C–0x4F     |                              |
| Numpad 0–9         | 0x62–0x6B| 0x0F, 0x1D...  | Full numpad mapping          |
| Numpad Enter       | 0x58     | 0x43          |                              |
| Numpad .           | 0x63     | 0x3C          |                              |

> Keys marked * are configurable overrides with no perfect Amiga equivalent.

### 4.2 Configuration Override Format

The config file may contain a `[keymap]` section with entries of the form:

```ini
[keymap]
# USB_HID_code = Amiga_keycode  (both in hex)
0x49 = 0x5F   ; Insert -> Help
```

---

## 5. Mouse Quadrature Output

### 5.1 Quadrature Encoding

The Amiga mouse interface uses two-phase quadrature encoding per axis. For each axis, two GPIO pins (A and B) produce a 90°-offset square wave. The direction of movement is determined by which phase leads.

```
Forward:   A: ─┐ ┌─┐ ┌─
           B: ──┐ ┌─┐ ┌
Backward:  A: ─┐ ┌─┐ ┌─
           B: ┐ ┌─┐ ┌──
```

Each full quadrature cycle (4 edges) = 1 count at the CIA chip.

### 5.2 Resolution and Pulse Rate

- Target: 1200 DPI
- Amiga CIA counts quadrature edges; maximum reliable count rate is approximately 6000 counts/second per axis (limited by CIA timer resolution at 0.709 MHz PAL / 0.715 MHz NTSC).
- At 1200 DPI, moving the mouse at 5 inches/second = 6000 counts/second — at the CIA limit.
- The firmware SHALL clamp the output pulse rate to a configurable maximum (default: 5500 counts/second) to avoid CIA overflow.
- USB HID mouse reports deliver delta-X and delta-Y values; the firmware accumulates these and generates quadrature pulses at the appropriate rate using a PIO state machine or timer-driven ISR on Core 1.

### 5.3 PIO Usage

Quadrature generation is well-suited to the RP2040's PIO. A single PIO state machine per axis can generate the two-phase output from a FIFO-fed count value, freeing Core 1 from bit-banging.

---

## 6. Joystick Port Output

### 6.1 Digital Mode

GPIO lines are driven low (active) or high-Z (inactive) to simulate open-collector joystick switches. A 74HC125 or similar buffer with output-enable provides the open-drain behaviour and 5 V tolerance.

### 6.2 Analogue Stick → Digital Mapping

Dead zone: configurable, default ±25% of full scale.  
Outside dead zone: the axis value is mapped to the corresponding directional output.  
Diagonal movement: both axes active simultaneously.

### 6.3 Analogue Stick → Quadrature Mode (stretch)

For games that read the joystick port as a mouse (e.g. some racing games), the analogue stick value can be scaled to a quadrature pulse rate and output on the joystick port quadrature lines. This mode is selected per-port in the config file.

---

## 7. Inter-Core Event Queue

### 7.1 Structure

Three ring buffers in shared SRAM, one per device type:

```c
typedef struct {
    uint8_t amiga_keycode;  // 7-bit keycode
    bool    key_down;       // true = press, false = release
} kbd_event_t;

typedef struct {
    int16_t dx;
    int16_t dy;
    uint8_t buttons;        // bitmask: bit0=left, bit1=right, bit2=middle
    int8_t  wheel;
} mouse_event_t;

typedef struct {
    uint8_t directions;     // bitmask: bit0=up, bit1=down, bit2=left, bit3=right
    uint8_t buttons;        // bitmask: bit0=fire1, bit1=fire2
} joystick_event_t;
```

### 7.2 Ring Buffer

- Capacity: 32 entries per buffer (power of 2 for cheap modulo)
- Write (Core 0): non-blocking; drops event if full (with debug counter)
- Read (Core 1): non-blocking; returns false if empty
- Synchronisation: `__dmb()` memory barriers; no mutex needed for single-producer single-consumer

---

## 8. Configuration File

### 8.1 Location and Format

File: `USB2AMIGA.CFG` in the root of the Pico's flash FAT filesystem (presented via USB MSC when config button held at boot).

Format: INI-style, case-insensitive keys, `#` comments.

### 8.2 Schema

```ini
[mouse]
max_pulse_rate = 5500       ; counts/sec, max CIA-safe rate
middle_button_action = middle ; middle | key:0x5F | none
scroll_wheel_action = scroll  ; scroll | key_up_down | none

[joystick]
port2_mode = digital          ; digital | quadrature
analogue_deadzone = 25        ; percent, 0-49

[keymap]
# USB_HID = Amiga  (hex values)
# 0x49 = 0x5F

[gamepad]
dpad_up    = joy_up
dpad_down  = joy_down
dpad_left  = joy_left
dpad_right = joy_right
button_a   = fire1
button_b   = fire2
button_x   = key:0x50        ; F1
button_y   = key:0x51        ; F2
```

---

## 9. Firmware Module Structure

```
src/
  usb2amiga.c          — main(), core0/core1 loops
  tusb_config.h        — TinyUSB configuration

  usb_hid_kbd.c/h      — USB HID keyboard input, keycode translation
  usb_hid_mouse.c/h    — USB HID mouse input
  usb_hid_gamepad.c/h  — USB HID gamepad input

  ami_kbd.c/h          — Amiga keyboard protocol output (bit-bang KCLK/KDAT)
  ami_mouse.c/h        — Amiga mouse quadrature output (PIO)
  ami_joystick.c/h     — Amiga joystick port output

  event_queue.c/h      — Lock-free ring buffers (kbd, mouse, joystick)
  keymap.c/h           — USB→Amiga keycode mapping table + config overrides
  config.c/h           — Config file parser (FAT + FatFs)
  pio_quadrature.pio   — PIO program for quadrature generation
```

---

## 10. PCB Specification (Outline)

### 10.1 Form Factor
- ATX shield PCB, sized to fit the A4000T ATX slot rear panel opening
- Two-board stack: main logic board + rear I/O panel board, connected by a right-angle header

### 10.2 Rear I/O Connectors
| Connector       | Type         | Notes                                  |
|-----------------|--------------|----------------------------------------|
| Keyboard        | 5-pin DIN    | To Amiga keyboard port                 |
| Mouse           | DB9 female   | Port 1 — mouse                         |
| Joystick        | DB9 female   | Port 2 — joystick/gamepad              |
| Serial          | DB25 male    | Passthrough only (v1)                  |
| Parallel        | DB25 female  | Passthrough only (v1)                  |
| USB-A (×1 min)  | USB-A female | USB host input; ×4 with hub (stretch)  |

### 10.3 Key ICs
| Component              | Purpose                                      |
|------------------------|----------------------------------------------|
| RP2040 (Pico module)   | Main controller                              |
| 74AHCT125 (×2)         | 3.3 V → 5 V level shift, open-drain outputs  |
| USB2514B or GL850G     | USB 2.0 hub (stretch goal)                   |
| MCP1700-3302 or similar| 5 V → 3.3 V LDO for Pico                    |
| PRTR5V0U2X (×per port) | ESD protection on all external connectors    |
| Polyfuse 500 mA        | USB 5 V supply protection                    |

### 10.4 Keyboard Passthrough (Stretch)
A second 5-pin DIN socket on the rear panel, wired in parallel with the Pico's KCLK/KDAT outputs via a multiplexer (e.g. 74HC157), allows an original Amiga keyboard to be connected. The Pico monitors both sources and arbitrates.

---

## 11. References

### Amiga Keyboard Protocol
- **Amiga Hardware Reference Manual (3rd ed., 1992)** — Appendix A "Keyboard Interface". Defines the KCLK/KDAT open-drain serial protocol, keycode byte format `(raw_keycode << 1) | updown`, 20 µs clock half-period, 143 ms ACK timeout, inter-key gap ≥ 200 µs, and the power-up sync byte 0xFF. Also defines the reset warning keycode 0x78 and the Ctrl+Left Amiga+Right Amiga combo.  
  Commodore-Amiga Inc., ISBN 0-553-35395-9.  
  Scanned copy archived at: https://archive.org/details/amiga-hardware-reference-manual-3rd-edition

- **Amiga keyboard protocol — community documentation**  
  https://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0173.html  
  Confirms keycode table, timing values, and ACK handshake behaviour. Cross-checked against HRM.

### Amiga Keycode Table
- **Amiga Hardware Reference Manual (3rd ed.)**, Appendix A, Table A-1 "Keyboard Raw Keycodes".  
  All Amiga raw keycode values in `keymap.c` are taken directly from this table.

### USB HID Keycodes
- **USB HID Usage Tables, Version 1.12** (2004), Section 10 "Keyboard/Keypad Page (0x07)".  
  USB Implementers Forum. https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf  
  All USB HID keycode values (0x04–0xE7) in `keymap.c` are taken from this document.

### Amiga Mouse / Quadrature Encoding
- **Amiga Hardware Reference Manual (3rd ed.)**, Chapter 7 "Joystick-Mouse Controller Chip (Denise/Lisa)".  
  Defines the two-phase quadrature encoding used on the DB9 mouse port, the CIA counter increment per quadrature edge, and the maximum reliable count rate.

- **Amiga CIA 8520 Datasheet** — MOS Technology / Commodore.  
  Confirms CIA timer clock rates: 0.709379 MHz (PAL) / 0.715909 MHz (NTSC), used to derive the maximum safe quadrature pulse rate of ~6000 counts/second.  
  Archived at: https://archive.org/details/mos-8520-cia-datasheet

- **NewMouse standard** — scroll wheel rawkey codes 0x7A (up) and 0x7B (down).  
  Defined by Olaf Barthel and Michael Sinz for AmigaOS 3.x.  
  https://wiki.amigaos.net/wiki/NewMouse_Standard

### RP2040 / Pico SDK
- **RP2040 Datasheet** (2021), Raspberry Pi Ltd.  
  https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf  
  PIO state machine architecture (section 3.4), multicore FIFO (section 2.3.1), memory barrier requirements.

- **Raspberry Pi Pico C/C++ SDK** (2021), Raspberry Pi Ltd.  
  https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf

### TinyUSB
- **TinyUSB documentation and source** — Ha Thach.  
  https://docs.tinyusb.org  
  USB host HID keyboard/mouse/gamepad API used throughout `usb_hid_*.c`.
