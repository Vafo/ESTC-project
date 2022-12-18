#ifndef MAIN_PICKER_STM_H
#define MAIN_PICKER_STM_H

#include "hsv_rgb.h"
#include "main_pwm.h"

typedef enum
{
    DISPLAY_MODE,
    HUE_MODIFICATION_MODE,
    SATURATION_MODIFICATION_MODE,
    BRIGHTNESS_MODIFICATION_MODE,

    PICKER_MODES_NUMBER
} picker_stm_mode_t;

typedef struct 
{
    hsv hsv;
    rgb rgb;
    picker_stm_mode_t cur_mode;
    hsv_idx_t cur_component;

    pwm_abs_cnxt_t *rgb_cnxt;
    pwm_abs_cnxt_t *led_cnxt;

    uint8_t updated;
    float set_period_to;
} picker_stm_cnxt_t;


void picker_stm_init();
void picker_stm_next_state();
void picker_stm_set_state(picker_stm_mode_t new_state);

// Rename to press_handler etc.
void picker_stm_on_press();
void picker_stm_on_release();
void picker_stm_on_double_click();

#endif