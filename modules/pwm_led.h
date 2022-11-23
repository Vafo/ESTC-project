#ifndef PWM_LED_H
#define PWM_LED_H

#include <stdint.h>
#include "app_util.h"

/** @brief PWM frequency in Hz. */
#define PWM_FREQ 1000
/** @brief PWM period in US. */
#define PWM_PERIOD_US ROUNDED_DIV(1000000, PWM_FREQ)
/** @brief PWM period in MS. */
#define PWM_PERIOD_MS ROUNDED_DIV(PWM_PERIOD_US, 1000)

/**
 * @brief Function for initializing PWM.
 */
void pwm_led_init(void);


/**
 * @brief Function for pulsing LED for period.
 * 
 * @param[in] pin Pin index.
 * @param[in] duty_cycle Duty cycle in range [0-1].
 */
void pwm_led_value(uint32_t pin, float duty_cycle);

#endif