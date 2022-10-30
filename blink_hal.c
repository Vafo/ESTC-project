#include "blink_hal.h"

static uint8_t leds_gpio[LEDS_NUMBER] = LEDS;

void led_init(void)
{
    for(uint8_t i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds_gpio[i]);
        led_off(i);
    }

}

void led_off(uint32_t led)
{
    ASSERT(led < LEDS_NUMBER);
    nrf_gpio_pin_write(leds_gpio[led], 1);
}

void led_on(uint32_t led)
{
    ASSERT(led < LEDS_NUMBER);
    nrf_gpio_pin_write(leds_gpio[led], 0);
}

void led_invert(uint32_t led)
{
    ASSERT(led < LEDS_NUMBER);
    nrf_gpio_pin_toggle(leds_gpio[led]);
}