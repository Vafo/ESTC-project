#ifndef BLINK_HAL_H
#define BLINK_HAL_H

#include <stdint.h>

#ifdef BOARD_PCA10059
    /** @brief Number of LEDs. */
    #define LEDS_NUMBER 4

    /** @brief Index of Yellow LED 1. */
    #define LED_1_IDX       0
    /** @brief Index of Red LED 2. */
    #define LED_2RED_IDX    1
    /** @brief Index of Green LED 2. */
    #define LED_2GREEN_IDX  2
    /** @brief Index of Blue LED 2. */
    #define LED_2BLUE_IDX   3
#else
    #error No Board specified
#endif

/**
 * @brief Function for initializing all LEDS.
 */
void led_init_all(void);

/**
 * @brief Function for initializing specific LED.
 *
 * @param[in] led LED index.
 */
void led_init(uint32_t led);

/**
 * @brief Function for enabling specific LED.
 *
 * @param[in] led LED index.
 */
void led_on(uint32_t led);

/**
 * @brief Function for disabling specific LED.
 *
 * @param[in] led LED index.
 */
void led_off(uint32_t led);

/**
 * @brief Function for inverting specific LED.
 *
 * @param[in] led LED index.
 */
void led_invert(uint32_t led);

/**
 * @brief Function for checking whether led is on.
 *
 * @param[in] led LED index.
 * 
 * @retval 1 Led is on.
 * @retval 0 Led is off.
 */
uint8_t led_is_on(uint32_t led);

/**
 * @brief Function for checking whether led is off.
 *
 * @param[in] led LED index.
 * 
 * @retval 1 Led is off.
 * @retval 0 Led is on.
 */
uint8_t led_is_off(uint32_t led);

#endif