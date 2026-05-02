/*
 * The MIT License (MIT)
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#include <stdio.h>
#include "tusb.h"
#include "event_queue.h"
#include "keymap.h"

/* Modifier bit → USB HID keycode */
static const uint8_t modifier_usb_codes[8] = {
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
};

static inline bool find_key(hid_keyboard_report_t const *r, uint8_t code) {
    for (int i = 0; i < 6; i++)
        if (r->keycode[i] == code) return true;
    return false;
}

static void push_key(uint8_t usb_code, bool key_down) {
    uint8_t amiga = keymap_usb_to_amiga(usb_code);
    if (amiga == AMIGA_KEY_NONE) return;
    kbd_event_t ev = { .amiga_keycode = amiga, .key_down = key_down };
    kbd_queue_push(&ev);
}

static void process_kbd_report(hid_keyboard_report_t const *new_r) {
    static hid_keyboard_report_t prev = {0, 0, {0}};

    uint8_t mod_changed = new_r->modifier ^ prev.modifier;
    for (int i = 0; i < 8; i++) {
        uint8_t bit = 1u << i;
        if (mod_changed & bit)
            push_key(modifier_usb_codes[i], (new_r->modifier & bit) != 0);
    }

    for (int i = 0; i < 6; i++)
        if (prev.keycode[i] && !find_key(new_r, prev.keycode[i]))
            push_key(prev.keycode[i], false);

    for (int i = 0; i < 6; i++)
        if (new_r->keycode[i] && !find_key(&prev, new_r->keycode[i]))
            push_key(new_r->keycode[i], true);

    prev = *new_r;
}

/* Called from the unified HID callbacks in usb2amiga.c */
void usb_hid_kbd_report(uint8_t const *report, uint16_t len) {
    if (len >= sizeof(hid_keyboard_report_t))
        process_kbd_report((hid_keyboard_report_t const *)report);
}
