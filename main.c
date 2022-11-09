
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "blink_hal.h"
#include "button_hal.h"

#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 

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

int flip;


// Extracted from nrf_gpiote.h:409 :_-(
// Since it is encapsulated originally, maybe user is not supposed to check
// whether the event is set or not
__STATIC_INLINE bool own_gpiote_event_is_set(nrfx_gpiote_pin_t pin)
{
    return (*(uint32_t *)nrfx_gpiote_in_event_addr_get(pin) == 0x1UL) ? true : false;
}

void sleeper()
{
    flip = 1;
    while(!own_gpiote_event_is_set(BUTTON_1) && flip)
        ;
}

void set_flip()
{
    flip = 0;
}

int main(void)
{
    
    led_init();
    nrfx_err_t error = db_event_init(BUTTON_1, set_flip, sleeper);
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
    sleeper();
    while(true)
    {
        
        for(int i = 0; i < LEDS_NUMBER; i++)
        {
            for(int j = 0; j < timing[i]; j++)
            {
                led_invert(i);
                nrf_delay_ms(BLINK_DURATION);
                led_invert(i);
                nrf_delay_ms(PAUSE_DURATION);
            }
        }
        
    }
    
}

#endif

/**
 *@}
 **/
