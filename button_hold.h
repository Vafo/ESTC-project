#ifndef BUTTON_HOLD_H
#define BUTTON_HOLD_H

#include "nrfx_gpiote.h"
#include "boards.h"

#define BLINK_DEBUG 1

#if BLINK_DEBUG == 0
#include "blink_hal.h"
#define debug_led_green_on() led_on(LED_2GREEN_IDX)
#define debug_led_green_off() led_off(LED_2GREEN_IDX)
#define debug_led_blue_on() led_on(LED_2BLUE_IDX)
#define debug_led_blue_off() led_off(LED_2BLUE_IDX)
#else
#define debug_led_green_on()
#define debug_led_green_off()
#define debug_led_blue_on()
#define debug_led_blue_off()
#endif

typedef void (*db_event_handler)(void);

// Extracted from nrf_gpiote.h:409 :_-(
// Since it is encapsulated originally, maybe user is not supposed to check
// whether the event is set or not
__STATIC_INLINE bool own_gpiote_event_is_set(nrfx_gpiote_pin_t pin)
{
    return (*(uint32_t *)nrfx_gpiote_in_event_addr_get(pin) == 0x1UL) ? true : false;
}

// A workaround for bug which happens at the very beginning

// Initialize button with events on release and press
nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release);

// Handler functions for double event
void db_on_release(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void db_on_press(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

#endif