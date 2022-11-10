#ifndef PWM_LED_H
#define PWM_LED_H

#include "nrfx_systick.h"
// #include "arm_math.h"

#define PWM_FREQ 1000   // Hz
#define PWM_PERIOD_US ROUNDED_DIV(1000000, PWM_FREQ) // us 
#define PWM_PERIOD_MS ROUNDED_DIV(PWM_PERIOD_US, 1000) // ms 

void pwm_led_init(void);
// value [0 - 1]
void pwm_led_value(uint32_t pin, float value);

#endif