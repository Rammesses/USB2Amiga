/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 */
#ifndef _USB_HID_KBD_H_
#define _USB_HID_KBD_H_
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void usb_hid_kbd_report(uint8_t const *report, uint16_t len);
#ifdef __cplusplus
}
#endif
#endif
