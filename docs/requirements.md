# USB2Amiga — Requirements Specification

Version: 0.2  
Date: 2026-04-28  
Author: Joel Hammond-Turner

---

## 1. Overview

USB2Amiga is a Raspberry Pi Pico-based adapter that allows modern USB keyboards, mice, and gamepads to be used with Amiga computers. It presents itself to the Amiga as a native keyboard, mouse, and joystick using the Amiga's original hardware protocols. It is packaged as a PCB stack in an ATX shield form factor suitable for installation in an A4000T case.

---

## 2. Target Hardware

### 2.1 Amiga Models
- Primary: Amiga 4000, Amiga 4000T
- Secondary: Amiga 2000, Amiga 3000

All target models use the same Amiga keyboard serial protocol. The physical connector differs by model (see technical spec section 2.1). Accelerator cards (e.g. Phase5 CyberStorm, Warp Engine, DKB Wildfire) replace the CPU module only; the CIA chips, keyboard port, and joystick/mouse ports are on the motherboard and are unaffected by accelerator installation. No special handling is required for accelerated systems.

### 2.2 Microcontroller
- Raspberry Pi Pico (RP2040)
- Dual-core ARM Cortex-M0+ at 133 MHz
- 264 KB SRAM, 2 MB on-board flash

### 2.3 Power Supply
- Pico powered from 5 V available on either:
  - The Amiga keyboard connector (+5 V pin)
  - Either DB9 joystick/mouse connector (pin 7: +5 V, 125 mA max per port, 200 mA surge)
- No USB bus power to Pico (Pico is USB host, not device)
- USB devices powered from Amiga 5 V rail via on-board polyfuse

---

## 3. Functional Requirements

### 3.1 USB Host

| ID | Requirement |
|----|-------------|
| USB-01 | The system SHALL act as a USB host using TinyUSB in host mode. |
| USB-02 | The system SHALL support USB HID keyboards. |
| USB-03 | The system SHALL support USB HID mice. |
| USB-04 | The system SHALL support USB HID gamepads. |
| USB-05 | The system SHALL support a USB hub, allowing keyboard, mouse, and gamepad to be connected simultaneously. |
| USB-06 | USB device connect and disconnect SHALL be handled gracefully without requiring a system reset. |
| USB-07 | The system SHOULD support up to 5 downstream USB devices (1 hub + 4 ports). |

### 3.2 Keyboard

| ID | Requirement |
|----|-------------|
| KBD-01 | The system SHALL implement the full Amiga keyboard serial protocol, including correct KDAT/KCLK timing and the ACK handshake pulse. |
| KBD-02 | The system SHALL implement the Amiga keyboard reset warning sequence (Ctrl+Left Amiga+Right Amiga), including the two-keycode handshake and the hard reset KCLK-low pulse. |
| KBD-03 | The system SHALL implement the full power-up sequence: sync, self-test result ($FD initiate / $FE terminate key stream). |
| KBD-04 | The system SHALL translate USB HID keycodes to Amiga keycodes using a configurable mapping table. |
| KBD-05 | The system SHALL support all standard Amiga keys including Help, Left Amiga, Right Amiga, and numeric keypad. |
| KBD-06 | The system SHALL support simultaneous key presses up to the USB HID 6-key rollover limit. |
| KBD-07 | The system SHALL correctly transmit key-down and key-up events. |
| KBD-08 | The system SHALL support modifier keys: Left/Right Shift, Ctrl, Left/Right Alt, Left/Right Amiga, Caps Lock. |
| KBD-09 | The keyboard protocol implementation SHALL be compatible with genuine Amiga hardware running at original clock speeds. |
| KBD-10 | The PCB SHALL provide the correct physical connector for each target model (6-pin Mini-DIN for A4000/A4000T; 5-pin DIN for A2000/A3000). |
| KBD-11 | A passthrough connector for an original Amiga keyboard SHOULD be provided as a stretch goal. |

### 3.3 Mouse

| ID | Requirement |
|----|-------------|
| MSE-01 | The system SHALL drive the Amiga mouse port (DB9 port 1) using quadrature-encoded pulse trains on the V, H, VQ, and HQ lines per the Amiga hardware specification. |
| MSE-02 | The system SHALL support a target resolution equivalent to 1200 DPI on a standard mouse mat. |
| MSE-03 | The system SHALL support left and right mouse buttons. |
| MSE-04 | The system SHALL support a middle mouse button, mapped to a configurable Amiga input. |
| MSE-05 | The system SHALL support a scroll wheel using the NewMouse standard: scroll up maps to rawkey $7A, scroll down to rawkey $7B, transmitted via the keyboard protocol channel. |
| MSE-06 | Mouse movement SHALL be smooth with no perceptible lag at normal desktop use speeds. |
| MSE-07 | The quadrature pulse rate SHALL be clamped to avoid counter overflow in the Amiga's Denise chip (max 127 counts per vertical blanking interval). |

### 3.4 Joystick / Gamepad

| ID | Requirement |
|----|-------------|
| JOY-01 | The system SHALL drive the Amiga joystick port (DB9 port 2) with digital forward/back/left/right/fire signals per the Amiga hardware specification. |
| JOY-02 | The system SHALL map a USB gamepad D-pad to the four Amiga joystick directions. |
| JOY-03 | The system SHALL support a second fire button. |
| JOY-04 | The system SHALL support analogue stick input from USB gamepads, with configurable mapping to either: (a) digital joystick directions with a configurable dead zone, or (b) quadrature mouse-style output on the joystick port. |
| JOY-05 | The gamepad mapping (button assignments, analogue mode) SHALL be configurable. |
| JOY-06 | Gamepad support is a required feature of the final product but is lower priority than keyboard and mouse. |

### 3.5 Configuration

| ID | Requirement |
|----|-------------|
| CFG-01 | Configuration SHALL be stored as a file on the Pico's internal flash, presented as a USB mass storage device when in config mode. |
| CFG-02 | The configuration file format SHALL be human-readable (INI format). |
| CFG-03 | Configuration SHALL include: USB keycode → Amiga keycode mapping overrides, mouse button assignments, scroll wheel action, gamepad button assignments, analogue stick mode and dead zone. |
| CFG-04 | The system SHALL load configuration at boot and apply it without recompilation. |
| CFG-05 | Invalid or missing configuration values SHALL fall back to compiled-in defaults without error. |
| CFG-06 | A physical button on the PCB SHALL trigger config mode (USB mass storage presentation). |

### 3.6 PCB / Hardware

| ID | Requirement |
|----|-------------|
| PCB-01 | The PCB SHALL be designed in an ATX shield form factor compatible with the A4000T case. |
| PCB-02 | The rear I/O panel SHALL expose: one keyboard connector (6-pin Mini-DIN for A4000/T; 5-pin DIN for A2000/3000 variant), two DB9 connectors (mouse port 1, joystick port 2), one DB25 parallel port connector, one DB25 serial port connector, and at least one USB-A host connector. |
| PCB-03 | The PCB SHOULD include an on-board USB hub circuit to support multiple USB-A host ports as a stretch goal. |
| PCB-04 | The PCB SHALL include appropriate level shifting between Pico 3.3 V GPIO and Amiga 5 V logic. |
| PCB-05 | The PCB SHALL include polyfuse protection on the USB 5 V supply to downstream devices. |
| PCB-06 | The PCB SHALL include ESD protection on all external connectors. |
| PCB-07 | The DB25 serial and parallel port connectors are physical passthrough only in v1 (no active logic required from the Pico). |

---

## 4. Non-Functional Requirements

| ID | Requirement |
|----|-------------|
| NFR-01 | End-to-end latency from USB keypress to Amiga keycode transmission SHALL be less than 5 ms under normal conditions. |
| NFR-02 | The firmware SHALL be written in C using the Pico SDK and TinyUSB. |
| NFR-03 | The firmware SHALL be open source under the MIT licence. |
| NFR-04 | The project SHALL include build instructions and a bill of materials. |
| NFR-05 | The firmware SHALL be structured to allow future support for additional Amiga models. |

---

## 5. Accelerator Card Compatibility

Common A4000/A4000T accelerator cards (Phase5 CyberStorm MkII/MkIII/PPC, Warp Engine, DKB Wildfire, GVP Tek) replace the CPU module via the CPU slot. They do not modify or replace the CIA chips, Denise, or the keyboard/joystick port circuitry. The USB2Amiga adapter interfaces exclusively with those unchanged circuits. No accelerator-specific handling is required.

---

## 6. Out of Scope (v1)

- Amiga serial port emulation (DB25 serial is passthrough only)
- Amiga parallel port emulation (DB25 parallel is passthrough only)
- CD32 gamepad protocol
- Amiga joystick port analogue paddle/potentiometer support
- Support for Amiga 500 / 600 / 1200 (different keyboard connector / form factor)
- AmigaOS 4 IECLASS_MOUSEWHEEL scroll wheel API (NewMouse standard covers OS 3.x)

---

## 7. References

- **Amiga Hardware Reference Manual (3rd ed., 1992)**, Commodore-Amiga Inc., ISBN 0-553-35395-9.  
  Primary source for keyboard protocol (Appendix A), mouse/joystick port pinouts and quadrature encoding (Chapter 7), and CIA counter behaviour.  
  https://archive.org/details/amiga-hardware-reference-manual-3rd-edition

- **USB HID Usage Tables, Version 1.12** (2004), USB Implementers Forum.  
  Source for all USB HID keycode values.  
  https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf

- **NewMouse Standard** — scroll wheel rawkey codes 0x7A/0x7B for AmigaOS 3.x.  
  https://wiki.amigaos.net/wiki/NewMouse_Standard

- **RP2040 Datasheet** (2021), Raspberry Pi Ltd.  
  https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf

- **Raspberry Pi Pico C/C++ SDK** (2021), Raspberry Pi Ltd.  
  https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf
