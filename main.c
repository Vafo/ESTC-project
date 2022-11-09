
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "blink_hal.h"
#include "button_hal.h"

#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 

#define SLEEP_FRACTION_MS 10


void id_to_led_blink(int num, int (*led_blink)[LEDS_NUMBER])
{
    int i;
    for(i = LEDS_NUMBER - 1; i >= 0 ; i--)
    {
        (*led_blink)[i] = num % 10;
        num /= 10;
    }
}

/**
 * @brief Function for application main entry.
 */
// #define BTN_PRESS
#define BTN_HOLD

#if defined(BTN_PRESS)

int main(void)
{
    // Configure board. 
    led_init();
    button_init();

    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, &timing);

    // Toggle LEDs.
    int i = 0,
        j = 0;
    while (true)
    {
        while(button_pressed())
        {
            led_invert(i);
            nrf_delay_ms(BLINK_DURATION);
            led_invert(i);
            nrf_delay_ms(PAUSE_DURATION);
            j++;
            if(j >= timing[i])
            {
                j = 0;
                i++;
            }
            if(i >= LEDS_NUMBER)
            {
                i = 0;
            }
        }
        
    }
    
}

#elif defined(BTN_HOLD)

#include "button_hold.h"

volatile int btn_released;


// Extracted from nrf_gpiote.h:409 :_-(
// Since it is encapsulated originally, maybe user is not supposed to check
// whether the event is set or not
__STATIC_INLINE bool own_gpiote_event_is_set(nrfx_gpiote_pin_t pin)
{
    return (*(uint32_t *)nrfx_gpiote_in_event_addr_get(pin) == 0x1UL) ? true : false;
}

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
    
    led_init();
    nrfx_err_t error = db_event_init(BUTTON_1, btn_released_off, btn_released_on);
    if(error != NRFX_SUCCESS)
    {
        while(true)
        {
            led_on(LED_2RED_IDX);
            nrf_delay_ms(250);
            led_off(LED_2RED_IDX);
            nrf_delay_ms(250);
        }
    }

    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, &timing);

    // Toggle LEDs.
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

#endif

/**
 *@}
 **/
