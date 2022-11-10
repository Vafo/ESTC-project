
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "math.h"

#include "blink_hal.h"
#include "button_hal.h"
#include "button_2events.h"
#include "pwm_led.h"


#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 // ms

#define SLEEP_FRACTION_MS 10

#define PI (float) 3.14159265359

#define DBL_CLICK_MARGIN 500 // ms

// Blinks error state infinitely
void blink_error()
{
    led_init(LED_2RED_IDX);
    while(true)
    {
        led_on(LED_2RED_IDX);
        nrf_delay_ms(250);
        led_off(LED_2RED_IDX);
        nrf_delay_ms(250);
    }
}

void id_to_led_blink(int num, int (*led_blink)[LEDS_NUMBER])
{
    int i;
    for(i = LEDS_NUMBER - 1; i >= 0 ; i--)
    {
        (*led_blink)[i] = num % 10;
        num /= 10;
    }
}


volatile int hold_blink;

void (*custom_blink)(uint32_t led, uint32_t period, uint32_t num_periods);

void custom_blink_wrapper(uint32_t led, uint32_t period, uint32_t num_periods)
{
    do
    {
        custom_blink(led, period, num_periods);
    } while (hold_blink);
}

void discrete_blink(uint32_t led, uint32_t period, uint32_t num_periods)
{
    uint32_t ms = period * PWM_PERIOD_MS;
    float value = (ms < BLINK_DURATION) ? 1 : 0;
    pwm_led_value(led, value);
}

void smooth_blink(uint32_t led, uint32_t period, uint32_t num_periods)
{
    float value = ((float) period / num_periods) * 2 * PI;
    pwm_led_value(led, sinf(value));
}

void on_press()
{
    custom_blink = discrete_blink;
}

void on_release()
{
    custom_blink = smooth_blink;
}

void on_boba()
{
    led_on(LED_2RED_IDX);
}

void out_boba()
{
    led_off(LED_2RED_IDX);
}

int main(void)
{
    
    led_init_all();
    pwm_led_init();
    nrfx_err_t error = db_event_init();
    if(error != NRFX_SUCCESS)
    {
        blink_error();
    }
    error = db_event_add(BUTTON_1, on_press, on_release);
    if(error != NRFX_SUCCESS)
    {
        blink_error();
    }

    error = db_event_add(BUTTON_1, on_boba, out_boba);
    if(error != NRFX_SUCCESS)
    {
        blink_error();
    }

    error = db_event_delete_handler(BUTTON_1, on_boba, out_boba);
    if(error != NRFX_SUCCESS)
    {
        blink_error();
    }

    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, &timing);
    custom_blink = smooth_blink;
    hold_blink = 0;

    uint32_t num_periods = ROUNDED_DIV(BLINK_DURATION + PAUSE_DURATION, PWM_PERIOD_MS);
    while(true)
    {
        
        for(int led = 0; led < LEDS_NUMBER; led++)
        {
            for(int led_blinks = 0; led_blinks < timing[led]; led_blinks++)
            {
                for(uint32_t period = 0; period < num_periods; period++)
                {
                    custom_blink_wrapper(led, period, num_periods);
                }
            }
        }
        
    }
    
}

/**
 *@}
 **/
