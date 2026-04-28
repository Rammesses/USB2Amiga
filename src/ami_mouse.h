/*
 * The MIT License (MIT)
 * Copyright (c) 2021 Joel Hammond-Turner
 */

#ifndef _AMI_MOUSE_H_
#define _AMI_MOUSE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ami_mouse_init(int *p_mouse_events);
void ami_mouse_out_task(void);

/* Override the default 5500 counts/sec CIA-safe pulse rate limit. */
void ami_mouse_set_max_pulse_rate(uint32_t counts_per_sec);

#ifdef __cplusplus
}
#endif

#endif /* _AMI_MOUSE_H_ */
