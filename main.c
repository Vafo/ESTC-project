
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "math.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"

#include "blink_hal.h"
#include "button_hal.h"
#include "button_2events.h"
#include "pwm_led.h"


#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 // ms

#define SLEEP_FRACTION_MS 10

#define PI (float) 3.14159265359
#define HOLD_MARGIN 500 // ms

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

/*
volatile int btn_released;

void btn_released_on()
{
    btn_released = 1;
}

void btn_released_off()
{
    btn_released = 0;
}

// FIXME ?
// This function would be valid only if sleep_ms is multiple of SLEEP_FRACTION_MS
//
// Notes: Maybe there is better way to wait for event to happen, and then unfreeze?

void progress_on_hold(uint32_t sleep_ms)
{
    int num_fractions = sleep_ms / SLEEP_FRACTION_MS;
    while(btn_released)
        ;
    for(int fraction_idx = 0; fraction_idx < num_fractions; fraction_idx++)
    {
        while(btn_released)
            ;
        nrf_delay_ms(SLEEP_FRACTION_MS);
    }
}
*/

void (*custom_blink)(uint32_t led, uint32_t period, uint32_t num_periods);
volatile int blink_hold;
void custom_blink_wrapper(uint32_t led, uint32_t period, uint32_t num_periods)
{
    do
    {
        custom_blink(led, period, num_periods);
    } while (blink_hold);
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

APP_TIMER_DEF(double_click_timer);
volatile int num_press;

static void lfclk_request()
{
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}

void double_click_timer_timeout()
{
    num_press = 0;
}

void on_press()
{
    if(num_press == 0)
    {
        app_timer_start(double_click_timer, APP_TIMER_TICKS(HOLD_MARGIN), NULL);
    }
    num_press++;
}

void on_release()
{
    if(num_press == 2)
    {
        app_timer_stop(double_click_timer);
        num_press = 0;
        blink_hold ^= 1;
    }
}


int main(void)
{
    
    led_init_all();
    pwm_led_init();
    nrfx_err_t error = db_event_init(BUTTON_1, on_press, on_release);
    if(error != NRFX_SUCCESS)
    {
        blink_error();
    }

    error = app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, double_click_timer_timeout);
    if(error != NRFX_SUCCESS)
    {
        blink_error();
    }
    blink_hold = 0;


    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, &timing);
    custom_blink = smooth_blink;

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

/*
int main(void)
{
    led_init_all();
    pwm_led_init();
    uint32_t num_cycles = ROUNDED_DIV(BLINK_DURATION + PAUSE_DURATION, PWM_PERIOD_MS);
    while(true)
    {
        for(int i = 0; i < num_cycles; i++)
        {
            float value = ((float) i / num_cycles) * 2 * PI;
            pwm_led_value(LED_2RED_IDX, sinf(value));
        }
        
    }
}
*/

/**
 *@}
 **/
