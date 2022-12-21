#ifndef MAIN_CLICK_H
#define MAIN_CLICK_H

#include "nrf_drv_clock.h"

#include "app_error.h"
#include "app_timer.h"

#include "button_hal.h"
#include "button_2events.h"

/**
 * @brief Function for initializing click events of main app.
 */
void main_click_init();
void on_press();
void on_release();
void on_double_click();

#endif