/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 *
 * Amiga keyboard protocol output.
 *
 * GPIO assignments (open-drain via external N-FET + 4.7k pull-up to 5V):
 *   KCLK  GPIO 0  — drive low or release (input/high-Z)
 *   KDAT  GPIO 1  — drive low or release
 *   /RST  GPIO 2  — input with pull-up, monitor only
 *
 * Protocol (Amiga HRM Appendix A):
 *   - Keycode byte = (raw_keycode << 1) | updown  (MSB first)
 *   - Each bit: set KDAT, then KCLK low 20 µs, KCLK high 20 µs
 *   - After 8 bits: wait for Amiga to pull KDAT low (ACK), timeout 143 ms
 *   - Inter-key gap: ≥ 200 µs
 *   - Power-up: send 0xFF sync byte, wait ACK
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "ami_kbd.h"
#include "event_queue.h"

/* GPIO pin numbers */
#define PIN_KCLK  0u
#define PIN_KDAT  1u
#define PIN_RESET 2u

/* Timing (µs) */
#define KCLK_HALF_US   20u
#define INTER_KEY_US   200u
#define ACK_TIMEOUT_US (143u * 1000u)

/* Amiga special keycodes */
#define AMIGA_SYNC_BYTE     0xFF
#define AMIGA_RESET_WARNING 0x78

/* Reset combo: Ctrl + Left Amiga + Right Amiga */
#define AMIGA_CTRL   0x63
#define AMIGA_LAMIGA 0x66
#define AMIGA_RAMIGA 0x67

/* ------------------------------------------------------------------ */
/* Open-drain helpers                                                   */
/* ------------------------------------------------------------------ */

/* Drive pin low (assert) */
static inline void pin_low(uint pin) {
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

/* Release pin (let pull-up take it high) */
static inline void pin_release(uint pin) {
    gpio_set_dir(pin, GPIO_IN);
}

/* Read pin state */
static inline bool pin_read(uint pin) {
    gpio_set_dir(pin, GPIO_IN);
    return gpio_get(pin);
}

/* ------------------------------------------------------------------ */
/* Low-level transmit                                                   */
/* ------------------------------------------------------------------ */

/*
 * Send one byte MSB-first using KCLK/KDAT.
 * Returns true if ACK received within timeout, false on timeout.
 */
static bool kbd_send_byte(uint8_t byte) {
    /* Shift out 8 bits, MSB first */
    for (int bit = 7; bit >= 0; bit--) {
        if (byte & (1u << bit))
            pin_release(PIN_KDAT); /* 1 = high */
        else
            pin_low(PIN_KDAT); /* 0 = low  */

        sleep_us(KCLK_HALF_US);
        pin_low(PIN_KCLK);
        sleep_us(KCLK_HALF_US);
        pin_release(PIN_KCLK);
    }

    /* Release KDAT so Amiga can pull it low for ACK */
    pin_release(PIN_KDAT);

    /* Wait for ACK: KDAT pulled low by Amiga */
    uint32_t deadline = time_us_32() + ACK_TIMEOUT_US;
    while (pin_read(PIN_KDAT)) {
        if (time_us_32() >= deadline) {
            printf("ami_kbd: ACK timeout\r\n");
            return false;
        }
    }
    /* Wait for Amiga to release KDAT */
    while (!pin_read(PIN_KDAT))
        ;

    return true;
}

/*
 * Encode and send an Amiga keycode event.
 * Wire byte = (amiga_keycode << 1) | (key_down ? 0 : 1)
 */
static bool kbd_send_key(uint8_t amiga_keycode, bool key_down) {
    uint8_t wire = (uint8_t)((amiga_keycode << 1) | (key_down ? 0u : 1u));
    bool ok = kbd_send_byte(wire);
    sleep_us(INTER_KEY_US);
    return ok;
}

/* ------------------------------------------------------------------ */
/* Reset warning detection                                              */
/* ------------------------------------------------------------------ */

static bool reset_keys_held[3]; /* [0]=ctrl [1]=lamiga [2]=ramiga */

static void update_reset_combo(uint8_t amiga_keycode, bool key_down) {
    if (amiga_keycode == AMIGA_CTRL)
        reset_keys_held[0] = key_down;
    else if (amiga_keycode == AMIGA_LAMIGA)
        reset_keys_held[1] = key_down;
    else if (amiga_keycode == AMIGA_RAMIGA)
        reset_keys_held[2] = key_down;
}

static bool reset_combo_active(void) {
    return reset_keys_held[0] && reset_keys_held[1] && reset_keys_held[2];
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void ami_kbd_init(int *p_kbd_events) {
    (void)p_kbd_events; /* queue is global; parameter kept for API compat */

    /* Initialise GPIOs as inputs (released / high-Z) initially */
    gpio_init(PIN_KCLK);
    gpio_init(PIN_KDAT);
    gpio_init(PIN_RESET);

    gpio_set_dir(PIN_KCLK, GPIO_IN);
    gpio_set_dir(PIN_KDAT, GPIO_IN);
    gpio_set_dir(PIN_RESET, GPIO_IN);
    gpio_pull_up(PIN_RESET);

    /* Power-up handshake: wait for KCLK high, then send sync byte */
    while (!gpio_get(PIN_KCLK))
        ; /* wait for Amiga to release clock */
    kbd_send_byte(AMIGA_SYNC_BYTE);
}

void ami_kbd_out_task(void) {
    kbd_event_t ev;
    if (!kbd_queue_pop(&ev))
        return;

    update_reset_combo(ev.amiga_keycode, ev.key_down);

    if (reset_combo_active()) {
        /* Send reset warning keycode */
        kbd_send_key(AMIGA_RESET_WARNING, true);
        kbd_send_key(AMIGA_RESET_WARNING, false);
        return;
    }

    kbd_send_key(ev.amiga_keycode, ev.key_down);
}
