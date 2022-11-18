#include "button_hal.h"

// Got to think about how to make it more general, for more custom buttons
// Currently it is restricted by the board
// What if external button is connected?

#ifdef BOARD_PCA10059
    #define BUTTONS_NUMBER 1
    #define BUTTON_1_PULL   NRF_GPIO_PIN_PULLUP

    #define BUTTONS {BUTTON_1}
    #define BUTTONS_PULL {BUTTON_1_PULL}

    #define BUTTON_PRESSED 0
#else
    #error No Board specified
#endif

static uint8_t buttons_gpio[BUTTONS_NUMBER] = BUTTONS;
static uint8_t buttons_pull[BUTTONS_NUMBER] = BUTTONS_PULL;

void button_init_all(void)
{
    static uint8_t buttons_initialized = 0;
    if(buttons_initialized == 0)
    {
        for(uint8_t i = 0; i < BUTTONS_NUMBER; i++)
        {
            button_init(buttons_gpio[i], buttons_pull[i]);
        }
    }
    buttons_initialized = 1;
}

void button_init(uint32_t pin, nrf_gpio_pin_pull_t pull_config)
{
    nrf_gpio_cfg_input(pin, pull_config);
}

uint8_t button_is_pressed(uint32_t pin)
{
    return nrf_gpio_pin_read(pin) == BUTTON_PRESSED;
}
