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

#include "tusb.h"

//--------------------------------------------------------------------+
// USB HID - Mouse
//--------------------------------------------------------------------+

CFG_TUSB_MEM_SECTION static hid_mouse_report_t usb_mouse_report;

void cursor_movement(int8_t x, int8_t y, int8_t wheel) {
    //------------- X -------------//
    if (x < 0) {
        printf(ANSI_CURSOR_BACKWARD(%d), (-x)); // move left        
    } else if (x > 0) {
        printf(ANSI_CURSOR_FORWARD(%d), x); // move right
    } else {}

    //------------- Y -------------//
    if (y < 0) {
        printf(ANSI_CURSOR_UP(%d), (-y)); // move up
    } else if (y > 0) {
        printf(ANSI_CURSOR_DOWN(%d), y); // move down
    } else {}

    //------------- wheel -------------//
    if (wheel < 0) {
        printf(ANSI_SCROLL_UP(%d), (-wheel)); // scroll up
    } else if (wheel > 0) {
        printf(ANSI_SCROLL_DOWN(%d), wheel); // scroll down
    } else {}

    if (x != 0 | y != 0 | wheel != 0)
    {
        // TODO: Write movement to the mouse movement buffer for the output task to 
    }
}

static inline void process_mouse_report(hid_mouse_report_t const *p_report) {
    static hid_mouse_report_t prev_report = {0};

    //------------- button state  -------------//
    uint8_t button_changed_mask = p_report->buttons ^prev_report.buttons;
    if (button_changed_mask & p_report->buttons) {
        printf(" %c%c%c ",
               p_report->buttons & MOUSE_BUTTON_LEFT ? 'L' : '-',
               p_report->buttons & MOUSE_BUTTON_MIDDLE ? 'M' : '-',
               p_report->buttons & MOUSE_BUTTON_RIGHT ? 'R' : '-');
    }

    //------------- cursor movement -------------//
    cursor_movement(p_report->x, p_report->y, p_report->wheel);
}


void tuh_hid_mouse_mounted_cb(uint8_t dev_addr) {
    // application set-up
    printf("A Mouse device (address %d) is mounted\r\n", dev_addr);
}

void tuh_hid_mouse_unmounted_cb(uint8_t dev_addr) {
    // application tear-down
    printf("A Mouse device (address %d) is unmounted\r\n", dev_addr);
}

// invoked ISR context
void tuh_hid_mouse_isr(uint8_t dev_addr, xfer_result_t event) {
    (void) dev_addr;
    (void) event;
}