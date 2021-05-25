/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/multicore.h"

void print_greeting(void);
void led_blinking_task(void);
extern void hid_task(void);

extern void ami_mouse_init(*p_mouse_events)
extern void ami_mouse_out_task(void);

extern void ami_kbd_init(*p_kbd_events);
extern void ami_kbd_out_task(void);

int main(void) {
    board_init();
    print_greeting();

    tusb_init();

    ami_kbd_init()
    multicore_launch_core1(ami_mouse_out_task);
    multicore_launch_core2(ami_kbd_out_task);

    while (1) {
        // tinyusb host task
        tuh_task();
        led_blinking_task();
        hid_task();
    }

    return 0;
}

void hid_task(void) {
    uint8_t const addr = 1;

    if (tuh_hid_keyboard_is_mounted(addr)) {
        if (!tuh_hid_keyboard_is_busy(addr)) {
            process_kbd_report(&usb_keyboard_report);
            tuh_hid_keyboard_get_report(addr, &usb_keyboard_report);
        }
    }
    if (tuh_hid_mouse_is_mounted(addr)) {
        if (!tuh_hid_mouse_is_busy(addr)) {
            process_mouse_report(&usb_mouse_report);
            tuh_hid_mouse_get_report(addr, &usb_mouse_report);
        }
    }
}


void led_blinking_task(void) {
    const uint32_t interval_ms = 250;
    static uint32_t start_ms = 0;

    static bool led_state = false;

    // Blink every interval ms
    if (board_millis() - start_ms < interval_ms) return; // not enough time
    start_ms += interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}

void print_greeting(void) {
    printf("This Host demo is configured to support:\n");
    if (CFG_TUH_HID_KEYBOARD) puts("  - HID Keyboard");
    if (CFG_TUH_HID_MOUSE) puts("  - HID Mouse");
}
