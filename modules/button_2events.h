#ifndef BUTTON_HOLD_H
#define BUTTON_HOLD_H

#include "nrfx_gpiote.h"
#include "main_logs.h"

#ifndef DB_HOLD_MARGIN
#define DB_HOLD_MARGIN 400 // ms

#endif

typedef void (*db_event_handler)(void);

/**
 * @brief Function for initializing button to trigger events on release and press
 * 
 * @param[in] pin GPIO pin.
 * @param[in] on_press On press event.
 * @param[in] on_release On release event.
 * @param[in] on_double_click On double click event.
 * 
 * @retval NRFX_SUCCESS Successfully intialized button and events.
 * @retval NRFX_ERROR_ Any kind of error.
 */
nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, 
                        db_event_handler on_press, 
                        db_event_handler on_release,
                        db_event_handler on_double_click);

#endif