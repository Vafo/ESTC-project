#ifndef BUTTON_2EVENTS_H
#define BUTTON_2EVENTS_H

#include "nrfx_gpiote.h"
#include "boards.h"

#define DB_EVENT_MAX_HANDLERS_PER_PIN 3
#define DB_EVENT_MAX_PINS 2

typedef void (*db_event_handler)(void);

// Initialize button with events on release and press
nrfx_err_t db_event_init();
nrfx_err_t db_event_add(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release);
nrfx_err_t db_event_delete_pin(nrfx_gpiote_pin_t pin);
nrfx_err_t db_event_delete_handler(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release);


#endif