/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#ifndef _KEYMAP_H_
#define _KEYMAP_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Sentinel: USB HID code has no Amiga equivalent */
#define AMIGA_KEY_NONE 0xFF

/*
 * Translate a USB HID keycode (0x00–0xE7) to an Amiga raw keycode.
 * Returns AMIGA_KEY_NONE if there is no mapping.
 * Modifier keys (0xE0–0xE7) are handled here too.
 */
uint8_t keymap_usb_to_amiga(uint8_t usb_code);

/*
 * Override a single mapping entry (used by config loader).
 * usb_code: 0x00–0xE7, amiga_code: 0x00–0x7F or AMIGA_KEY_NONE.
 */
void keymap_set_override(uint8_t usb_code, uint8_t amiga_code);

#ifdef __cplusplus
}
#endif

#endif /* _KEYMAP_H_ */
