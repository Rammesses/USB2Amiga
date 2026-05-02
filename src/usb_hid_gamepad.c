/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 *
 * Gamepad report handler — stub for Phase 5.
 */
#include <stdio.h>
#include "usb_hid_gamepad.h"

void usb_hid_gamepad_report(uint8_t const *report, uint16_t len) {
    (void)report; (void)len;
    /* Phase 5: parse report, push joystick_event_t to joystick_queue */
}
