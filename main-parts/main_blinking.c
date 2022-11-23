#include "main_blinking.h"

int period_cur;
int period_hold;
int blink_hold;
void (*custom_blink)(uint32_t led, uint32_t period, uint32_t num_periods);

void blink_error()
{
    led_init(LED_2RED_IDX);
    while(1)
    {
        led_on(LED_2RED_IDX);
        nrf_delay_ms(250);
        led_off(LED_2RED_IDX);
        nrf_delay_ms(250);
    }
}

void id_to_led_blink(int num, int *dest, int size)
{
    int i;
    for(i = size - 1; i >= 0 ; i--)
    {
        dest[i] = num % 10;
        num /= 10;
    }
}

void smooth_blink(uint32_t led, uint32_t period, uint32_t num_periods)
{
    float value = ((float) period_cur / num_periods) * 2 * PI;
    pwm_led_value(led, sinf(value));
}

void discrete_blink(uint32_t led, uint32_t period, uint32_t num_periods)
{
    uint32_t ms = period * PWM_PERIOD_MS;
    float value = ((float) period_cur / num_periods) * 2 * PI;
    value = (ms < BLINK_DURATION) ? sinf(value) : 0;
    pwm_led_value(led, value);
}

void custom_blink_wrapper(uint32_t led, uint32_t period, uint32_t num_periods)
{
    if(!period_hold)
    {
        period_cur = period;
    }

    do
    {
        custom_blink(led, period, num_periods);
    } while (blink_hold);
}