#include "main_blinking.h"

int intensity_cur;
int intensity_hold;
int blink_hold;
void (*custom_blink_f)(uint32_t led, uint32_t tick, uint32_t num_ticks);

#define CHANGE_HOLD_TICKS 5
#define ABS(X) ( (X) < 0 ? (-(X)) : (X))

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

void smooth_blink(uint32_t led, uint32_t tick, uint32_t num_ticks)
{
    float value = ((float) intensity_cur / num_ticks) * 2 * PI;
    pwm_led_value(led, ABS(sinf(value)));
}

void discrete_blink(uint32_t led, uint32_t tick, uint32_t num_ticks)
{
    uint32_t ms = tick * PWM_PERIOD_MS;
    float value = ((float) intensity_cur / num_ticks) * 2 * PI;
    value = (ms < BLINK_DURATION) ? ABS(sinf(value)) : 0;
    pwm_led_value(led, value);
}

void vary_intensity(uint32_t led, uint32_t tick, uint32_t num_ticks)
{
    intensity_hold = 0;
    while(!intensity_hold)
    {
        while (intensity_cur < num_ticks && !intensity_hold)
        {
            // Maybe I need to use goto, so that avoid that much of !intensity_hold ?
            for(int i = 0; i < CHANGE_HOLD_TICKS && !intensity_hold; i++)
            {
                smooth_blink(led,tick, num_ticks);
            }

            intensity_cur++;
        }
        if(intensity_cur >= num_ticks)
        {
            intensity_cur = 0;
        }
    }
}

// Make 2 separate busy loops
void custom_blink_wrapper(uint32_t led, uint32_t tick, uint32_t num_ticks)
{
    do
    {
        custom_blink_f(led, tick, num_ticks);
    } while (blink_hold);
}