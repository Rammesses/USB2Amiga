/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#ifndef _EVENT_QUEUE_H_
#define _EVENT_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Event types                                                          */
/* ------------------------------------------------------------------ */

typedef struct {
    uint8_t amiga_keycode;  /* 7-bit Amiga raw keycode                */
    bool    key_down;       /* true = press, false = release           */
} kbd_event_t;

typedef struct {
    int16_t dx;
    int16_t dy;
    uint8_t buttons;        /* bit0=left, bit1=right, bit2=middle      */
    int8_t  wheel;
} mouse_event_t;

typedef struct {
    uint8_t directions;     /* bit0=up, bit1=down, bit2=left, bit3=right */
    uint8_t buttons;        /* bit0=fire1, bit1=fire2                  */
} joystick_event_t;

/* ------------------------------------------------------------------ */
/* Generic ring-buffer (capacity must be a power of 2, max 32)         */
/* ------------------------------------------------------------------ */

#define QUEUE_CAPACITY 32u

/* One queue instance per event type — declare with QUEUE_DEFINE.      */
#define QUEUE_DEFINE(name, type)                                        \
    typedef struct {                                                    \
        type     buf[QUEUE_CAPACITY];                                   \
        volatile uint32_t head;                                         \
        volatile uint32_t tail;                                         \
    } name##_queue_t;                                                   \
    extern name##_queue_t name##_queue;                                 \
    static inline bool name##_queue_push(const type *ev) {             \
        uint32_t next = (name##_queue.head + 1u) & (QUEUE_CAPACITY-1u);\
        if (next == name##_queue.tail) return false; /* full */         \
        name##_queue.buf[name##_queue.head] = *ev;                      \
        __dmb();                                                        \
        name##_queue.head = next;                                       \
        return true;                                                    \
    }                                                                   \
    static inline bool name##_queue_pop(type *ev) {                    \
        if (name##_queue.tail == name##_queue.head) return false;       \
        *ev = name##_queue.buf[name##_queue.tail];                      \
        __dmb();                                                        \
        name##_queue.tail = (name##_queue.tail + 1u) & (QUEUE_CAPACITY-1u); \
        return true;                                                    \
    }

QUEUE_DEFINE(kbd,      kbd_event_t)
QUEUE_DEFINE(mouse,    mouse_event_t)
QUEUE_DEFINE(joystick, joystick_event_t)

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_QUEUE_H_ */
