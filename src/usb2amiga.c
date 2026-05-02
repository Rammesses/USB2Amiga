/*
 * The MIT License (MIT)
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#include <stdio.h>
#include "bsp/board.h"
#include "pico/multicore.h"
#include "tusb.h"

#include "ami_joystick.h"
#include "ami_kbd.h"
#include "ami_mouse.h"
#include "usb_hid_gamepad.h"
#include "usb_hid_kbd.h"
#include "usb_hid_mouse.h"

static int p_joystick_events;
static int p_kbd_events;
static int p_mouse_events;

static void led_blinking_task(void) {
    const uint32_t interval_ms = 250;
    static uint32_t start_ms = 0;
    static bool led_state = false;
    if (board_millis() - start_ms < interval_ms) return;
    start_ms += interval_ms;
    board_led_write(led_state);
    led_state = !led_state;
}

static void core1_output_loop(void) {
    while (1) {
        ami_kbd_out_task();
        ami_mouse_out_task();
        ami_joystick_out_task();
        led_blinking_task();
    }
}

int main(void) {
    board_init();

    printf("USB2Amiga starting\r\n");
    if (CFG_TUH_HID)   puts("  - HID (keyboard/mouse/gamepad)");

    tusb_init();

    ami_joystick_init(&p_joystick_events);
    ami_kbd_init(&p_kbd_events);
    ami_mouse_init(&p_mouse_events);

    multicore_launch_core1(core1_output_loop);

    while (1) {
        tuh_task();
    }
}

/* ------------------------------------------------------------------ */
/* TinyUSB 1.x unified HID host callbacks                              */
/* ------------------------------------------------------------------ */

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance,
                      uint8_t const *desc_report, uint16_t desc_len)
{
    (void)desc_report; (void)desc_len;
    uint8_t proto = tuh_hid_interface_protocol(dev_addr, instance);
    const char *name = (proto == HID_ITF_PROTOCOL_KEYBOARD) ? "keyboard" :
                       (proto == HID_ITF_PROTOCOL_MOUSE)    ? "mouse"    : "HID";
    printf("%s mounted (addr %d inst %d)\r\n", name, dev_addr, instance);

    /* Request first report */
    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID unmounted (addr %d inst %d)\r\n", dev_addr, instance);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                 uint8_t const *report, uint16_t len)
{
    uint8_t proto = tuh_hid_interface_protocol(dev_addr, instance);
    switch (proto) {
        case HID_ITF_PROTOCOL_KEYBOARD: usb_hid_kbd_report(report, len);     break;
        case HID_ITF_PROTOCOL_MOUSE:    usb_hid_mouse_report(report, len);   break;
        default:                        usb_hid_gamepad_report(report, len); break;
    }
    /* Re-arm for next report */
    tuh_hid_receive_report(dev_addr, instance);
}
