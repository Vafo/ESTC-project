#ifndef BUTTON_HAL_H
#define BUTTON_HAL_H

#include <stdint.h>
#include "nrf_gpio.h"

#ifdef BOARD_PCA10059
    /** @brief Number of Buttons. */
    #define BUTTONS_NUMBER 1
    /** @brief Index of first Button. */
    #define BUTTON_1_IDX   0
    /** @brief GPIO pin number of first Button. */
    #define BUTTON_1        NRF_GPIO_PIN_MAP(1,6)
    
#else
    #error No Board specified
#endif

/**
 * @brief Function for initializing all Buttons.
 */
void button_init_all(void);

/**
 * @brief Function for initializing specific Button.
 *
 * @param[in] pin GPIO pin.
 * @param[in] pull_config Pull configuration.
 */
void button_init(uint32_t pin, nrf_gpio_pin_pull_t pull_config);

/**
 * @brief Function for checking if button is pressed.
 *
 * @param[in] pin GPIO pin.
 * 
 * @retval 1 Button is pressed
 * @retval 0 Button is not pressed
 */
uint8_t button_is_pressed(uint32_t pin);

#endif