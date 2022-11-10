#ifndef BLINK_HAL_H
#define BLINK_HAL_H

#include <stdint.h>
#include "boards.h"

#ifdef BOARD_PCA10059
    #define LED_1_IDX       0
    #define LED_2RED_IDX    1
    #define LED_2GREEN_IDX  2
    #define LED_2BLUE_IDX   3
#endif

void led_init_all(void);
void led_init(uint32_t led);
void led_on(uint32_t led);
void led_off(uint32_t led);
void led_invert(uint32_t led);

#endif