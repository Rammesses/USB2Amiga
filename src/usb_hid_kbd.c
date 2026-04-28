/*
 * The MIT License (MIT)
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#include <stdio.h>
#include "tusb.h"
#include "event_queue.h"
#include "keymap.h"

/* Modifier bit positions in hid_keyboard_report_t.modifier */
#define MOD_LCTRL   (1u << 0)
#define MOD_LSHIFT  (1u << 1)
#define MOD_LALT    (1u << 2)
#define MOD_LGUI    (1u << 3)
#define MOD_RCTRL   (1u << 4)
#define MOD_RSHIFT  (1u << 5)
#define MOD_RALT    (1u << 6)
#define MOD_RGUI    (1u << 7)

/* USB modifier bit → USB HID keycode (so we can reuse the keymap table) */
static const uint8_t modifier_usb_codes[8] = {
    0xE0, /* LCTRL  */
    0xE1, /* LSHIFT */
    0xE2, /* LALT   */
    0xE3, /* LGUI   */
    0xE4, /* RCTRL  */
    0xE5, /* RSHIFT */
    0xE6, /* RALT   */
    0xE7, /* RGUI   */
};

CFG_TUSB_MEM_SECTION static hid_keyboard_report_t usb_keyboard_report;

static inline bool find_key_in_report(hid_keyboard_report_t const *r, uint8_t code) {
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

    /* --- Modifier changes --- */
    uint8_t mod_changed = new_r->modifier ^ prev.modifier;
    for (int i = 0; i < 8; i++) {
        uint8_t bit = 1u << i;
        if (mod_changed & bit)
            push_key(modifier_usb_codes[i], (new_r->modifier & bit) != 0);
    }

    /* --- Key releases (in prev but not in new) --- */
    for (int i = 0; i < 6; i++) {
        if (prev.keycode[i] && !find_key_in_report(new_r, prev.keycode[i]))
            push_key(prev.keycode[i], false);
    }

    /* --- Key presses (in new but not in prev) --- */
    for (int i = 0; i < 6; i++) {
        if (new_r->keycode[i] && !find_key_in_report(&prev, new_r->keycode[i]))
            push_key(new_r->keycode[i], true);
    }

    prev = *new_r;
}

void usb_hid_kbd_task(void) {
    uint8_t const addr = 1;
    if (tuh_hid_keyboard_is_mounted(addr) && !tuh_hid_keyboard_is_busy(addr)) {
        process_kbd_report(&usb_keyboard_report);
        tuh_hid_keyboard_get_report(addr, &usb_keyboard_report);
    }
}

void tuh_hid_keyboard_mounted_cb(uint8_t dev_addr) {
    printf("Keyboard mounted (addr %d)\r\n", dev_addr);
    tuh_hid_keyboard_get_report(dev_addr, &usb_keyboard_report);
}

void tuh_hid_keyboard_unmounted_cb(uint8_t dev_addr) {
    printf("Keyboard unmounted (addr %d)\r\n", dev_addr);
}

void tuh_hid_keyboard_isr(uint8_t dev_addr, xfer_result_t event) {
    (void)dev_addr; (void)event;
}
