/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#include "event_queue.h"

/* Storage for the three queues — zero-initialised by default */
kbd_queue_t kbd_queue;
mouse_queue_t mouse_queue;
joystick_queue_t joystick_queue;
