#ifndef MAIN_BLINKING_H
#define MAIN_BLINKING_H

#include "nrf_delay.h"

#include "blink_hal.h"
#include "pwm_led.h"

#include "math.h"

#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 // ms

#define PI (float) 3.14159265359

extern int period_cur;
extern int period_hold;
extern int blink_hold;
extern void (*custom_blink)(uint32_t led, uint32_t period, uint32_t num_periods);

void blink_error();
void id_to_led_blink(int num, int *dest, int size);
void smooth_blink(uint32_t led, uint32_t period, uint32_t num_periods);
void discrete_blink(uint32_t led, uint32_t period, uint32_t num_periods);
void custom_blink_wrapper(uint32_t led, uint32_t period, uint32_t num_periods);

#endif