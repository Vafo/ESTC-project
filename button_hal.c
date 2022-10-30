#include "button_hal.h"


void button_init(void)
{
    nrf_gpio_cfg_input(BTN_GPIO, BTN_PULL);
}

uint32_t button_pressed(void)
{
    return nrf_gpio_pin_read(BTN_GPIO) ? 0 : 1;
}
