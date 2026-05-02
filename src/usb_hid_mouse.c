/*
 * The MIT License (MIT)
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#include "tusb.h"
#include "event_queue.h"

static void process_mouse_report(hid_mouse_report_t const *r) {
    static hid_mouse_report_t prev = {0};

    if (r->x || r->y || r->wheel || (r->buttons != prev.buttons)) {
        mouse_event_t ev = {
            .dx      = r->x,
            .dy      = r->y,
            .buttons = r->buttons,
            .wheel   = r->wheel,
        };
        mouse_queue_push(&ev);
    }
    prev = *r;
}

/* Called from the unified HID callbacks in usb2amiga.c */
void usb_hid_mouse_report(uint8_t const *report, uint16_t len) {
    if (len >= sizeof(hid_mouse_report_t))
        process_mouse_report((hid_mouse_report_t const *)report);
}
