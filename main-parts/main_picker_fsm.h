#ifndef MAIN_PICKER_fsm_H
#define MAIN_PICKER_fsm_H

#include "hsv_rgb.h"
#include "main_pwm.h"

typedef enum
{
    DISPLAY_MODE,
    HUE_MODIFICATION_MODE,
    SATURATION_MODIFICATION_MODE,
    BRIGHTNESS_MODIFICATION_MODE,

    PICKER_MODES_NUMBER
} picker_fsm_mode_t;

typedef struct 
{
    hsv hsv;
    rgb rgb;
    picker_fsm_mode_t cur_mode;
    hsv_idx_t cur_component;

    pwm_abs_ctx_t *rgb_ctx;
    pwm_abs_ctx_t *led_ctx;

    uint8_t updated;
    float set_period_to;
} picker_fsm_ctx_t;


void picker_fsm_init();
void picker_fsm_next_state();
void picker_fsm_set_state(picker_fsm_mode_t new_state);

// Rename to press_handler etc.
void picker_fsm_press_handler();
void picker_fsm_release_handler();
void picker_fsm_double_click_handler();

void picker_fsm_save_hsv(hsv *src);
void picker_fsm_set_hsv(hsv *src);
void picker_fsm_get_hsv();

#endif