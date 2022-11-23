#include "pwm_led.h"
#include "nrfx_systick.h"
#include "blink_hal.h"

static nrfx_systick_state_t cur_state;

void pwm_led_init(void)
{
    nrfx_systick_init();
}

void pwm_led_value(uint32_t led_idx, float duty_cycle)
{
    if(duty_cycle > 1)
    {
        duty_cycle = 1;
    } 
    else if(duty_cycle < 0)
    {
        duty_cycle = 0;
    }

    uint32_t active_duty = (uint32_t) (duty_cycle * PWM_PERIOD_US);
    uint32_t inactive_duty = PWM_PERIOD_US - active_duty;
    nrfx_systick_get(&cur_state);
    led_on(led_idx);
    while(!nrfx_systick_test(&cur_state, active_duty))
        ;
    nrfx_systick_get(&cur_state);
    led_off(led_idx);
    while(!nrfx_systick_test(&cur_state, inactive_duty))
        ;

}