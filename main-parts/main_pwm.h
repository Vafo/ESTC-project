#ifndef MAIN_PWM_LED_H
#define MAIN_PWM_LED_H

#include <stdint.h>
#include "app_util.h"
#include "pwm_abs.h"

// <0=> 16 MHz 
// <1=> 8 MHz 
// <2=> 4 MHz 
// <3=> 2 MHz 
// <4=> 1 MHz 
// <5=> 500 kHz 
// <6=> 250 kHz 
// <7=> 125 kHz 

/** @brief PWM Base clock index. */
#define MAIN_PWM_BASE_CLOCK 7

typedef float (*main_pwm_math_fn)(float input, uint32_t top_value);

extern pwm_abs_ctx_t rgb_ctx;
extern pwm_abs_ctx_t led_ctx;

extern main_pwm_math_fn rgb_math_fn;
extern main_pwm_math_fn led_math_fn;

extern uint8_t func_hold;


// Input [0 - 1]
// Output 0
float off_func(float input, uint32_t top_value);

// Input [0 - 1]
// Output 1
float on_func(float input, uint32_t top_value);

// Input [0 - 1]
// Output [0 - top_value]
float sine_arc_func(float input, uint32_t top_value);

// Input [0 - 1]
// Output [0 - top_value]
float saw_func(float input, uint32_t top_value);

// Input [0 - 1]
// Output [0 - top_value]
float stair_func(float input, uint32_t top_value);

/**
 * @brief Function for initializing PWM.
 */
void main_pwm_init(pwm_abs_update_handler rgb_handler, pwm_abs_update_handler led_handler);




#endif