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

#include "ami_joystick.h"
#include "ami_kbd.h"
#include "ami_mouse.h"
#include "usb_hid_gamepad.h"
#include "usb_hid_kbd.h"
#include "usb_hid_mouse.h"

void print_greeting(void);
void led_blinking_task(void);
void core0_input_loop(void);
void core1_output_loop(void);

extern void hid_task(void);

static int p_joystick_events;
static int p_kbd_events;
static int p_mouse_events;

int main(void) {
    board_init();
    print_greeting();
    tusb_init();

    ami_joystick_init(&p_joystick_events);
    ami_kbd_init(&p_kbd_events);
    ami_mouse_init(&p_mouse_events);

    // start our two loop processes
    multicore_launch_core1(core1_output_loop);
    core0_input_loop();

    return 0;
}

void core0_input_loop(void)
{
    while (1) {
        // tinyusb host task
        tuh_task();
    }
}

void core1_output_loop(void)
{
    while (1) {
        // keyboard event handling
        ami_kbd_out_task();

        // mouse event handling
        ami_mouse_out_task();

        // gamepad / joystick handling
        ami_joystick_out_task();

        // watchdog blink         
        led_blinking_task();
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
    printf("AmiUSB vMAJ.MIN\n======\n  The flexible USB keyboard, mouse & gamepad interface\n\n");
    printf("AmiUSB - Copyright (c) 2021 Joel Hammond-Turner (github.com/Rammesses/AmiUSB)\n");
    printf("TinyUSB - Copyright (c) 2019 Ha Thach (tinyusb.org)\n");
    printf("\nBuild MAJ.MIN.PCH.BLD - configured for:\n");
    if (CFG_TUH_HID_KEYBOARD) puts("  - HID Keyboard\n");
    if (CFG_TUH_HID_MOUSE) puts("  - HID Mouse\n");
    if (CFG_TUH_HID_GAMEPAD) puts("  - HID Gamepad\n");
}
