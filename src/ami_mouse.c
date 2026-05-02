/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#include "ami_mouse.h"
#include "event_queue.h"

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pio_quadrature.pio.h"

/* ------------------------------------------------------------------ */
/* GPIO assignments                                                     */
/* ------------------------------------------------------------------ */
#define GPIO_H_A       6 /* Horizontal quadrature A                 */
#define GPIO_H_B       8 /* Horizontal quadrature B — NOT consecutive*/
#define GPIO_V_A       7 /* Vertical quadrature A                   */
#define GPIO_V_B       9 /* Vertical quadrature B — NOT consecutive */
#define GPIO_BTN_MID   10
#define GPIO_BTN_LEFT  11
#define GPIO_BTN_RIGHT 12

/*
 * NOTE: The spec assigns H-A=GPIO6, H-B=GPIO8 (non-consecutive) and
 * V-A=GPIO7, V-B=GPIO9 (non-consecutive). The PIO OUT pins base must
 * be consecutive. We therefore use two separate PIO state machines,
 * each with OUT base = A pin, and manually set the B pin via side-set
 * — but side-set would require rewriting the PIO program.
 *
 * Simpler solution: use the PIO for the A pin only (1-bit OUT), and
 * mirror the B pin from Core 1 by writing it via gpio_put() in the
 * same step. The PIO handles the delay; Core 1 sets B before pushing
 * the next word.
 *
 * Actually the cleanest solution given non-consecutive pins: don't use
 * PIO OUT for 2 pins. Instead use PIO with 1 OUT pin (A) and drive B
 * directly from Core 1 before each push. The PIO word then only needs
 * the A value + delay. B is set by Core 1 before the push.
 *
 * Revised word format (1 OUT pin, LSB-first):
 *   bit  0    — A pin value
 *   bits 31:1 — delay count
 */

/* ------------------------------------------------------------------ */
/* Quadrature Gray-code table                                           */
/* phase: 0..3, forward = increment, backward = decrement (mod 4)      */
/* entry: { A, B }                                                      */
/* ------------------------------------------------------------------ */
static const uint8_t QUAD_A[4] = {1, 1, 0, 0};
static const uint8_t QUAD_B[4] = {0, 1, 1, 0};

/* ------------------------------------------------------------------ */
/* PIO state                                                            */
/* ------------------------------------------------------------------ */
static PIO pio_inst;
static uint sm_h; /* horizontal axis state machine               */
static uint sm_v; /* vertical axis state machine                 */
static uint pio_offset;

/* Current quadrature phase for each axis */
static uint8_t phase_h = 0;
static uint8_t phase_v = 0;

/* Max pulse rate (steps/sec) — default 5500 */
static uint32_t max_pulse_rate = 5500;

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

/*
 * Push one quadrature step to a state machine.
 * Sets the B pin directly (non-consecutive), then pushes a word to the
 * PIO TX FIFO encoding the A pin value and the inter-step delay.
 *
 * delay_cycles: number of system clock cycles for the step period.
 * The PIO loop burns (delay_cycles - 4) iterations (4 cycles overhead).
 */
static void push_step(uint sm, uint gpio_a, uint gpio_b, uint8_t a_val, uint8_t b_val,
                      uint32_t delay_cycles) {
    gpio_put(gpio_b, b_val);

    /* Word: bit 0 = A value, bits 31:1 = delay count */
    uint32_t loop_count = (delay_cycles > 4) ? (delay_cycles - 4) : 0;
    uint32_t word = ((uint32_t)a_val & 1u) | (loop_count << 1);
    pio_sm_put_blocking(pio_inst, sm, word);
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void ami_mouse_init(int *p_mouse_events) {
    (void)p_mouse_events; /* legacy parameter, unused */

    /* Button GPIOs — output, active low, default high (released) */
    gpio_init(GPIO_BTN_LEFT);
    gpio_set_dir(GPIO_BTN_LEFT, GPIO_OUT);
    gpio_put(GPIO_BTN_LEFT, 1);
    gpio_init(GPIO_BTN_RIGHT);
    gpio_set_dir(GPIO_BTN_RIGHT, GPIO_OUT);
    gpio_put(GPIO_BTN_RIGHT, 1);
    gpio_init(GPIO_BTN_MID);
    gpio_set_dir(GPIO_BTN_MID, GPIO_OUT);
    gpio_put(GPIO_BTN_MID, 1);

    /* B pins driven directly — init as GPIO output */
    gpio_init(GPIO_H_B);
    gpio_set_dir(GPIO_H_B, GPIO_OUT);
    gpio_put(GPIO_H_B, QUAD_B[phase_h]);
    gpio_init(GPIO_V_B);
    gpio_set_dir(GPIO_V_B, GPIO_OUT);
    gpio_put(GPIO_V_B, QUAD_B[phase_v]);

    /* Load PIO program */
    pio_inst = pio0;
    pio_offset = pio_add_program(pio_inst, &quadrature_program);

    /* Horizontal SM — OUT base = GPIO_H_A, 1 OUT pin */
    sm_h = pio_claim_unused_sm(pio_inst, true);
    pio_sm_config cfg_h = quadrature_program_get_default_config(pio_offset);
    sm_config_set_out_pins(&cfg_h, GPIO_H_A, 1);
    sm_config_set_out_shift(&cfg_h, true, false, 32); /* LSB-first, no autopull */
    pio_gpio_init(pio_inst, GPIO_H_A);
    pio_sm_set_consecutive_pindirs(pio_inst, sm_h, GPIO_H_A, 1, true);
    pio_sm_init(pio_inst, sm_h, pio_offset, &cfg_h);
    pio_sm_set_pins(pio_inst, sm_h, QUAD_A[phase_h]);
    pio_sm_set_enabled(pio_inst, sm_h, true);

    /* Vertical SM — OUT base = GPIO_V_A, 1 OUT pin */
    sm_v = pio_claim_unused_sm(pio_inst, true);
    pio_sm_config cfg_v = quadrature_program_get_default_config(pio_offset);
    sm_config_set_out_pins(&cfg_v, GPIO_V_A, 1);
    sm_config_set_out_shift(&cfg_v, true, false, 32); /* LSB-first, no autopull */
    pio_gpio_init(pio_inst, GPIO_V_A);
    pio_sm_set_consecutive_pindirs(pio_inst, sm_v, GPIO_V_A, 1, true);
    pio_sm_init(pio_inst, sm_v, pio_offset, &cfg_v);
    pio_sm_set_pins(pio_inst, sm_v, QUAD_A[phase_v]);
    pio_sm_set_enabled(pio_inst, sm_v, true);
}

void ami_mouse_set_max_pulse_rate(uint32_t counts_per_sec) {
    if (counts_per_sec > 0 && counts_per_sec <= 6000)
        max_pulse_rate = counts_per_sec;
}

void ami_mouse_out_task(void) {
    mouse_event_t ev;
    if (!mouse_queue_pop(&ev))
        return;

    /* Button outputs — active low */
    gpio_put(GPIO_BTN_LEFT, (ev.buttons & 0x01) ? 0 : 1);
    gpio_put(GPIO_BTN_RIGHT, (ev.buttons & 0x02) ? 0 : 1);
    gpio_put(GPIO_BTN_MID, (ev.buttons & 0x04) ? 0 : 1);

    if (ev.dx == 0 && ev.dy == 0)
        return;

    /*
     * Compute step period in system clock cycles.
     * We pace both axes at the same rate (the faster axis determines it).
     * step_period = sys_clk / max_pulse_rate
     */
    uint32_t sys_clk = clock_get_hz(clk_sys);
    uint32_t step_cycles = sys_clk / max_pulse_rate;

    /*
     * Interleave H and V steps so diagonal movement is smooth rather
     * than sequential (L-shaped). Both FIFOs are filled in lockstep;
     * since each SM runs independently, they execute in parallel.
     */
    int dir_h = (ev.dx >= 0) ? 1 : -1;
    int steps_h = (ev.dx >= 0) ? ev.dx : -ev.dx;
    int dir_v = (ev.dy >= 0) ? 1 : -1;
    int steps_v = (ev.dy >= 0) ? ev.dy : -ev.dy;
    int total = (steps_h > steps_v) ? steps_h : steps_v;

    for (int i = 0; i < total; i++) {
        if (i < steps_h) {
            phase_h = (phase_h + 4 + dir_h) & 3;
            push_step(sm_h, GPIO_H_A, GPIO_H_B, QUAD_A[phase_h], QUAD_B[phase_h], step_cycles);
        }
        if (i < steps_v) {
            phase_v = (phase_v + 4 + dir_v) & 3;
            push_step(sm_v, GPIO_V_A, GPIO_V_B, QUAD_A[phase_v], QUAD_B[phase_v], step_cycles);
        }
    }
}
