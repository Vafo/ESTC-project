#include "button_hold.h"

nrfx_gpiote_in_config_t  conf_release = {
    .sense = NRF_GPIOTE_POLARITY_LOTOHI,    // Release
    .pull = NRF_GPIO_PIN_PULLUP,
    .is_watcher = false,
    .hi_accuracy = 1,
    .skip_gpio_setup = false
};

nrfx_gpiote_in_config_t  conf_press = {
    .sense = NRF_GPIOTE_POLARITY_HITOLO,
    .pull = NRF_GPIO_PIN_PULLUP,
    .is_watcher = false,
    .hi_accuracy = 1,
    .skip_gpio_setup = false
};

void (*db_event_user_on_press)(void);
void (*db_event_user_on_release)(void);

void db_on_press(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    debug_led_green_on();
    nrfx_gpiote_in_uninit(pin);
    nrfx_gpiote_in_init(pin, &conf_release, db_on_release);
    nrfx_gpiote_in_event_enable(pin, true);

    db_event_user_on_press();

}

void db_on_release(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    debug_led_blue_off();
    nrfx_gpiote_in_uninit(pin);
    nrfx_gpiote_in_init(pin, &conf_press, db_on_press);
    nrfx_gpiote_in_event_enable(pin, true);

    db_event_user_on_release();

}

nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release)
{
    if(!nrfx_gpiote_is_init())
    {
        nrfx_gpiote_init();
    }
    
    nrfx_err_t err_code1 = nrfx_gpiote_in_init(BUTTON_1, &conf_press, db_on_press);
    // Configure board. 

    NRFX_ASSERT(on_press != NULL)
    db_event_user_on_press = on_press;
    NRFX_ASSERT(on_release != NULL)
    db_event_user_on_release = on_release;

    nrfx_gpiote_in_event_enable(BUTTON_1, true);

    return err_code1;
}

