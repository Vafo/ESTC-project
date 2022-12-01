#ifndef MAIN_BLINKING_H
#define MAIN_BLINKING_H

#include "nrf_delay.h"

#include "blink_hal.h"
#include "main_pwm.h"

#include "math.h"

#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 // ms

#define PI (float) 3.14159265359

extern int intensity_cur;
extern int intensity_hold;
extern int blink_hold;
extern void (*custom_blink_f)(uint32_t led, uint32_t tick, uint32_t num_ticks);

void blink_error();
void id_to_led_blink(int num, int *dest, int size);
void smooth_blink(uint32_t led, uint32_t tick, uint32_t num_ticks);
void discrete_blink(uint32_t led, uint32_t tick, uint32_t num_ticks);
void vary_intensity(uint32_t led, uint32_t tick, uint32_t num_ticks);
void custom_blink_wrapper(uint32_t led, uint32_t tick, uint32_t num_ticks);

#endif