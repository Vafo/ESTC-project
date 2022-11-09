#include "blink_hal.h"

#ifdef BOARD_PCA10059
    #define LED_1_GPIO       NRF_GPIO_PIN_MAP(0,6)
    #define LED_2R_GPIO      NRF_GPIO_PIN_MAP(0,8)
    #define LED_2G_GPIO      NRF_GPIO_PIN_MAP(1,9)
    #define LED_2B_GPIO      NRF_GPIO_PIN_MAP(0,12)
    
    #define LEDS {LED_1_GPIO, LED_2R_GPIO, LED_2G_GPIO, LED_2B_GPIO}
#endif

static uint8_t leds_gpio[LEDS_NUMBER] = LEDS;

void led_init_all(void)
{
    static uint8_t leds_initialized = 0;
    if(leds_initialized == 0)
    {
        for(uint8_t i = 0; i < LEDS_NUMBER; i++)
        {
            led_init(i);
        }
    }
    leds_initialized = 1;
}

void led_init(uint32_t led)
{
    nrf_gpio_cfg_output(leds_gpio[led]);
    led_off(led);
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