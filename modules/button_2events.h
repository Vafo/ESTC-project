#ifndef BUTTON_HOLD_H
#define BUTTON_HOLD_H

#include "nrfx_gpiote.h"


typedef void (*db_event_handler)(void);

/**
 * @brief Function for initializing button to trigger events on release and press
 * 
 * @param[in] pin GPIO pin.
 * @param[in] on_press On press event.
 * @param[in] on_release On release event.
 * 
 * @retval NRFX_SUCCESS Successfully intialized button and events.
 * @retval NRFX_ERROR_ Any kind of error.
 */
nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release);


#endif