#ifndef BUTTON_HOLD_H
#define BUTTON_HOLD_H

#include "nrfx_gpiote.h"
#include "boards.h"

typedef void (*db_event_handler)(void);

// Initialize button with events on release and press
nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release);

// Handler functions for double event
void db_on_release(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void db_on_press(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

#endif