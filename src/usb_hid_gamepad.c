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

#include <tusb.h>

//--------------------------------------------------------------------+
// USB HID - gamepad
//--------------------------------------------------------------------+

// CFG_TUSB_MEM_SECTION static hid_gamepad_report_t usb_gamepad_report;

// static inline void process_kbd_report(hid_gamepad_report_t const *p_new_report) {
//     static hid_gamepad_report_t prev_report = {0, 0, {0}}; // previous report to check key released

//     prev_report = *p_new_report;
// }

void usb_hid_gamepad_task(void) {
    // uint8_t const addr = 1;

    // if (tuh_hid_gamepad_is_mounted(addr)) {
    //     if (!tuh_hid_gamepad_is_busy(addr)) {
    //         process_kbd_report(&usb_gamepad_report);
    //         tuh_hid_gamepad_get_report(addr, &usb_gamepad_report);
    //     }
    // }
}

void tuh_hid_gamepad_mounted_cb(uint8_t dev_addr) {
    // application set-up
    printf("A gamepad device (address %d) is mounted\r\n", dev_addr);
    // tuh_hid_gamepad_get_report(dev_addr, &usb_gamepad_report);
}

void tuh_hid_gamepad_unmounted_cb(uint8_t dev_addr) {
    // application tear-down
    printf("A gamepad device (address %d) is unmounted\r\n", dev_addr);
}

// invoked ISR context
void tuh_hid_gamepad_isr(uint8_t dev_addr, xfer_result_t event) {
    (void) dev_addr;
    (void) event;
}
