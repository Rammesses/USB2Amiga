/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 *
 * USB HID keycode → Amiga raw keycode lookup table.
 *
 * Table covers USB HID Usage Page 0x07 (Keyboard/Keypad), codes 0x00–0xE7.
 * Modifier keys (0xE0–0xE7) are appended at the end of the table.
 * AMIGA_KEY_NONE (0xFF) means no Amiga equivalent.
 *
 * Amiga keycode byte sent on the wire = (amiga_code << 1) | updown_bit.
 * This table stores only the 7-bit raw keycode; the caller adds the
 * direction bit.
 *
 * Sources:
 *   Amiga Hardware Reference Manual, Appendix A (keyboard matrix)
 *   USB HID Usage Tables 1.12, Section 10
 */

#include "keymap.h"

#define N AMIGA_KEY_NONE

/*
 * Index = USB HID keycode (0x00–0xE7).
 * Value = Amiga raw keycode (7-bit), or N if unmapped.
 *
 * The table is split into two parts:
 *   [0x00..0xDF]  regular keys
 *   [0xE0..0xE7]  modifier keys (stored at offset 0xE0 in the same array)
 */
static uint8_t keymap_table[0xE8] = {
    /* 0x00 */ N,     /* Reserved (no event)                          */
    /* 0x01 */ N,     /* Keyboard ErrorRollOver                       */
    /* 0x02 */ N,     /* Keyboard POSTFail                            */
    /* 0x03 */ N,     /* Keyboard ErrorUndefined                      */
    /* 0x04 */ 0x20,  /* A                                            */
    /* 0x05 */ 0x35,  /* B                                            */
    /* 0x06 */ 0x33,  /* C                                            */
    /* 0x07 */ 0x22,  /* D                                            */
    /* 0x08 */ 0x12,  /* E                                            */
    /* 0x09 */ 0x23,  /* F                                            */
    /* 0x0A */ 0x24,  /* G                                            */
    /* 0x0B */ 0x25,  /* H                                            */
    /* 0x0C */ 0x17,  /* I                                            */
    /* 0x0D */ 0x26,  /* J                                            */
    /* 0x0E */ 0x27,  /* K                                            */
    /* 0x0F */ 0x28,  /* L                                            */
    /* 0x10 */ 0x37,  /* M                                            */
    /* 0x11 */ 0x36,  /* N                                            */
    /* 0x12 */ 0x18,  /* O                                            */
    /* 0x13 */ 0x19,  /* P                                            */
    /* 0x14 */ 0x10,  /* Q                                            */
    /* 0x15 */ 0x13,  /* R                                            */
    /* 0x16 */ 0x21,  /* S                                            */
    /* 0x17 */ 0x14,  /* T                                            */
    /* 0x18 */ 0x16,  /* U                                            */
    /* 0x19 */ 0x34,  /* V                                            */
    /* 0x1A */ 0x11,  /* W                                            */
    /* 0x1B */ 0x32,  /* X                                            */
    /* 0x1C */ 0x15,  /* Y                                            */
    /* 0x1D */ 0x31,  /* Z                                            */
    /* 0x1E */ 0x01,  /* 1 / !                                        */
    /* 0x1F */ 0x02,  /* 2 / @                                        */
    /* 0x20 */ 0x03,  /* 3 / #                                        */
    /* 0x21 */ 0x04,  /* 4 / $                                        */
    /* 0x22 */ 0x05,  /* 5 / %                                        */
    /* 0x23 */ 0x06,  /* 6 / ^                                        */
    /* 0x24 */ 0x07,  /* 7 / &                                        */
    /* 0x25 */ 0x08,  /* 8 / *                                        */
    /* 0x26 */ 0x09,  /* 9 / (                                        */
    /* 0x27 */ 0x0A,  /* 0 / )                                        */
    /* 0x28 */ 0x44,  /* Return                                       */
    /* 0x29 */ 0x45,  /* Escape                                       */
    /* 0x2A */ 0x41,  /* Backspace                                    */
    /* 0x2B */ 0x42,  /* Tab                                          */
    /* 0x2C */ 0x40,  /* Space                                        */
    /* 0x2D */ 0x0B,  /* - / _                                        */
    /* 0x2E */ 0x0C,  /* = / +                                        */
    /* 0x2F */ 0x1A,  /* [ / {                                        */
    /* 0x30 */ 0x1B,  /* ] / }                                        */
    /* 0x31 */ 0x0D,  /* \ / |  (US layout backslash)                 */
    /* 0x32 */ 0x0D,  /* # / ~  (ISO layout, same physical key as \)  */
    /* 0x33 */ 0x29,  /* ; / :                                        */
    /* 0x34 */ 0x2A,  /* ' / "                                        */
    /* 0x35 */ 0x00,  /* ` / ~  (grave accent — Amiga key left of 1)  */
    /* 0x36 */ 0x38,  /* , / <                                        */
    /* 0x37 */ 0x39,  /* . / >                                        */
    /* 0x38 */ 0x3A,  /* / / ?                                        */
    /* 0x39 */ 0x62,  /* Caps Lock                                    */
    /* 0x3A */ 0x50,  /* F1                                           */
    /* 0x3B */ 0x51,  /* F2                                           */
    /* 0x3C */ 0x52,  /* F3                                           */
    /* 0x3D */ 0x53,  /* F4                                           */
    /* 0x3E */ 0x54,  /* F5                                           */
    /* 0x3F */ 0x55,  /* F6                                           */
    /* 0x40 */ 0x56,  /* F7                                           */
    /* 0x41 */ 0x57,  /* F8                                           */
    /* 0x42 */ 0x58,  /* F9                                           */
    /* 0x43 */ 0x59,  /* F10                                          */
    /* 0x44 */ 0x4B,  /* F11 → Amiga numpad (                         */
    /* 0x45 */ 0x4C,  /* F12 → Amiga numpad )                         */
    /* 0x46 */ 0x5F,  /* Print Screen → Help                          */
    /* 0x47 */ N,     /* Scroll Lock (no Amiga equivalent)            */
    /* 0x48 */ N,     /* Pause (no Amiga equivalent)                  */
    /* 0x49 */ 0x66,  /* Insert → Left Amiga (configurable)           */
    /* 0x4A */ 0x6F,  /* Home (configurable)                          */
    /* 0x4B */ 0x6C,  /* Page Up (configurable)                       */
    /* 0x4C */ 0x46,  /* Delete                                       */
    /* 0x4D */ 0x6F,  /* End (configurable)                           */
    /* 0x4E */ 0x6D,  /* Page Down (configurable)                     */
    /* 0x4F */ 0x4E,  /* Right arrow                                  */
    /* 0x50 */ 0x4F,  /* Left arrow                                   */
    /* 0x51 */ 0x4D,  /* Down arrow                                   */
    /* 0x52 */ 0x4C,  /* Up arrow                                     */
    /* 0x53 */ N,     /* Num Lock / Clear                             */
    /* 0x54 */ 0x5C,  /* Numpad /                                     */
    /* 0x55 */ 0x5D,  /* Numpad *                                     */
    /* 0x56 */ 0x4A,  /* Numpad -                                     */
    /* 0x57 */ 0x5E,  /* Numpad +                                     */
    /* 0x58 */ 0x43,  /* Numpad Enter                                 */
    /* 0x59 */ 0x1D,  /* Numpad 1 / End                               */
    /* 0x5A */ 0x1E,  /* Numpad 2 / Down                              */
    /* 0x5B */ 0x1F,  /* Numpad 3 / PgDn                              */
    /* 0x5C */ 0x2D,  /* Numpad 4 / Left                              */
    /* 0x5D */ 0x2E,  /* Numpad 5                                     */
    /* 0x5E */ 0x2F,  /* Numpad 6 / Right                             */
    /* 0x5F */ 0x3D,  /* Numpad 7 / Home                              */
    /* 0x60 */ 0x3E,  /* Numpad 8 / Up                                */
    /* 0x61 */ 0x3F,  /* Numpad 9 / PgUp                              */
    /* 0x62 */ 0x0F,  /* Numpad 0 / Ins                               */
    /* 0x63 */ 0x3C,  /* Numpad . / Del                               */
    /* 0x64 */ 0x30,  /* ISO \ / |  (key right of left shift, ISO)    */
    /* 0x65 */ N,     /* Application (Menu key)                       */
    /* 0x66 */ N,     /* Power                                        */
    /* 0x67 */ N,     /* Numpad =                                     */
    /* 0x68 */ N,     /* F13                                          */
    /* 0x69 */ N,     /* F14                                          */
    /* 0x6A */ N,     /* F15                                          */
    /* 0x6B */ N,     /* F16                                          */
    /* 0x6C */ N,     /* F17                                          */
    /* 0x6D */ N,     /* F18                                          */
    /* 0x6E */ N,     /* F19                                          */
    /* 0x6F */ N,     /* F20                                          */
    /* 0x70 */ N,     /* F21                                          */
    /* 0x71 */ N,     /* F22                                          */
    /* 0x72 */ N,     /* F23                                          */
    /* 0x73 */ N,     /* F24                                          */
    /* 0x74 */ N,     /* Execute                                      */
    /* 0x75 */ N,     /* Help                                         */
    /* 0x76 */ N,     /* Menu                                         */
    /* 0x77 */ N,     /* Select                                       */
    /* 0x78 */ N,     /* Stop                                         */
    /* 0x79 */ N,     /* Again                                        */
    /* 0x7A */ N,     /* Undo                                         */
    /* 0x7B */ N,     /* Cut                                          */
    /* 0x7C */ N,     /* Copy                                         */
    /* 0x7D */ N,     /* Paste                                        */
    /* 0x7E */ N,     /* Find                                         */
    /* 0x7F */ N,     /* Mute                                         */
    /* 0x80 */ N,     /* Volume Up                                    */
    /* 0x81 */ N,     /* Volume Down                                  */
    /* 0x82..0xDF */ N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N, /* 0x82-0x8F */
                     N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N, /* 0x90-0x9F */
                     N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N, /* 0xA0-0xAF */
                     N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N, /* 0xB0-0xBF */
                     N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N, /* 0xC0-0xCF */
                     N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N, /* 0xD0-0xDF */
    /* Modifier keys 0xE0–0xE7 */
    /* 0xE0 */ 0x63,  /* Left Ctrl                                    */
    /* 0xE1 */ 0x60,  /* Left Shift                                   */
    /* 0xE2 */ 0x64,  /* Left Alt                                     */
    /* 0xE3 */ 0x66,  /* Left GUI (Win/Cmd) → Left Amiga              */
    /* 0xE4 */ 0x63,  /* Right Ctrl (same as Left Ctrl on Amiga)      */
    /* 0xE5 */ 0x61,  /* Right Shift                                  */
    /* 0xE6 */ 0x65,  /* Right Alt                                    */
    /* 0xE7 */ 0x67,  /* Right GUI → Right Amiga                      */
};

#undef N

uint8_t keymap_usb_to_amiga(uint8_t usb_code) {
    if (usb_code >= sizeof(keymap_table)) return AMIGA_KEY_NONE;
    return keymap_table[usb_code];
}

void keymap_set_override(uint8_t usb_code, uint8_t amiga_code) {
    if (usb_code < sizeof(keymap_table))
        keymap_table[usb_code] = amiga_code;
}
