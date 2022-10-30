#ifndef BLINK_HAL_H
#define BLINK_HAL_H

#include <stdint.h>
#include "boards.h"

#ifdef BOARD_PCA10059
    #define LED_1_IDX       0
    #define LED_2RED_IDX    1
    #define LED_2GREEN_IDX  2
    #define LED_2BLUE_IDX   3

    #define LED_1_GPIO       NRF_GPIO_PIN_MAP(0,6)
    #define LED_2R_GPIO      NRF_GPIO_PIN_MAP(0,8)
    #define LED_2G_GPIO      NRF_GPIO_PIN_MAP(1,9)
    #define LED_2B_GPIO      NRF_GPIO_PIN_MAP(0,12)

    #define LEDS {LED_1_GPIO, LED_2R_GPIO, LED_2G_GPIO, LED_2B_GPIO}
#endif

void led_init(void);
void led_on(uint32_t led);
void led_off(uint32_t led);
void led_invert(uint32_t led);

#endif