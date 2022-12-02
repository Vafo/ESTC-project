#ifndef PWM_LED_H
#define PWM_LED_H

#include <stdint.h>
#include "app_util.h"

// <0=> 16 MHz 
// <1=> 8 MHz 
// <2=> 4 MHz 
// <3=> 2 MHz 
// <4=> 1 MHz 
// <5=> 500 kHz 
// <6=> 250 kHz 
// <7=> 125 kHz 

/** @brief PWM Base clock index. */
#define MAIN_PWM_BASE_CLOCK 4

/** @brief PWM Top value. */
#define MAIN_PWM_TOP_VALUE 1000
/** @brief Function duration in ms*/
#define MAIN_PWM_FNC_DURATION 10000

/** @brief PWM Base clock frequency in Hz. */
#define MAIN_PWM_FREQ ((16000000) / (1 << MAIN_PWM_BASE_CLOCK)) 
// /** @brief PWM period in US. */
// #define MAIN_PWM_PERIOD_US (((float) 1000000) / MAIN_PWM_FREQ)
/** @brief PWM period in MS. */
#define MAIN_PWM_PERIOD_MS ((((float) 1000) / MAIN_PWM_FREQ) * MAIN_PWM_TOP_VALUE)


extern uint8_t func_hold;

/**
 * @brief Function for initializing PWM.
 */
void pwm_led_init(void);

void pwm_led_value(uint32_t pin, float duty_cycle);

/**
 * @brief Function for pulsing LED for period.
 * 
 * @param[in] pin Pin index.
 * @param[in] duty_cycle Duty cycle in range [0-1].
 */
void pwm_led_individual_value(uint32_t pin, float duty_cycle);
void pwm_led_individual_value_and_wait(uint32_t led_idx, float duty_cycle);

#endif