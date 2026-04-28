/*
 * The MIT License (MIT)
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#include <stdio.h>
#include "tusb.h"
#include "event_queue.h"

CFG_TUSB_MEM_SECTION static hid_mouse_report_t usb_mouse_report;

static void process_mouse_report(hid_mouse_report_t const *r) {
    static hid_mouse_report_t prev = {0};

    mouse_event_t ev = {
        .dx      = r->x,
        .dy      = r->y,
        .buttons = r->buttons,
        .wheel   = r->wheel,
    };

    /* Only push if something changed */
    if (r->x || r->y || r->wheel || (r->buttons != prev.buttons))
        mouse_queue_push(&ev);

    prev = *r;
}

void usb_hid_mouse_task(void) {
    uint8_t const addr = 1;
    if (tuh_hid_mouse_is_mounted(addr) && !tuh_hid_mouse_is_busy(addr)) {
        process_mouse_report(&usb_mouse_report);
        tuh_hid_mouse_get_report(addr, &usb_mouse_report);
    }
}

void tuh_hid_mouse_mounted_cb(uint8_t dev_addr) {
    printf("Mouse mounted (addr %d)\r\n", dev_addr);
}

void tuh_hid_mouse_unmounted_cb(uint8_t dev_addr) {
    printf("Mouse unmounted (addr %d)\r\n", dev_addr);
}

void tuh_hid_mouse_isr(uint8_t dev_addr, xfer_result_t event) {
    (void)dev_addr; (void)event;
}
