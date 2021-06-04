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
#include "pico/multicore.h"
#include "tusb.h"

#include "ami_kbd.h"
#include "ami_mouse.h"
#include "usb_hid_kbd.h"
#include "usb_hid_mouse.h"

void print_greeting(void);
void led_blinking_task(void);
extern void hid_task(void);

static int p_kbd_events;

int main(void) {
    board_init();
    print_greeting();

    tusb_init();

    ami_kbd_init(&p_kbd_events);

    multicore_launch_core1(ami_mouse_out_task);
    multicore_launch_core1(ami_kbd_out_task);

    while (1) {
        // tinyusb host task
        tuh_task();
        led_blinking_task();
        usb_hid_kbd_task();
        usb_hid_mouse_task();
    }

    return 0;
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
