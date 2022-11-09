
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "blink_hal.h"
#include "button_hal.h"
#include "button_hold.h"


#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 

#define SLEEP_FRACTION_MS 10

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

int main(void)
{
    
    led_init_all();
    nrfx_err_t error = db_event_init(BUTTON_1, btn_released_off, btn_released_on);
    if(error != NRFX_SUCCESS)
    {
        blink_error();
    }

    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, &timing);

    // Workaround for the very first button press
    btn_released_on();
    progress_on_hold(0);
    while(true)
    {
        
        for(int i = 0; i < LEDS_NUMBER; i++)
        {
            for(int j = 0; j < timing[i]; j++)
            {
                led_invert(i);
                progress_on_hold(BLINK_DURATION);
                led_invert(i);
                progress_on_hold(PAUSE_DURATION);
            }
        }
        
    }
    
}


/**
 *@}
 **/
