#include "button_2events.h"
#include "drv_rtc.h"

#define BLINK_DEBUG 0

#if BLINK_DEBUG == 1
#include "blink_hal.h"
#define debug_led_green_on() led_on(LED_2GREEN_IDX)
#define debug_led_green_off() led_off(LED_2GREEN_IDX)
#define debug_led_blue_on() led_on(LED_2BLUE_IDX)
#define debug_led_blue_off() led_off(LED_2BLUE_IDX)
#else
#define debug_led_green_on()
#define debug_led_green_off()
#define debug_led_blue_on()
#define debug_led_blue_off()
#endif


nrfx_gpiote_in_config_t  conf_toggle = {
    .sense = NRF_GPIOTE_POLARITY_TOGGLE,
    .pull = NRF_GPIO_PIN_PULLUP,
    .is_watcher = false,
    .hi_accuracy = 1,
    .skip_gpio_setup = false
};

void (*db_event_user_on_press)(void);
void (*db_event_user_on_release)(void);



void db_on_toggle(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if(nrf_gpio_pin_read(pin) == 0)
    {
        debug_led_blue_on();
        db_event_user_on_press();
    }
    else
    {
        debug_led_blue_off();
        db_event_user_on_release();
    }
    
}

nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release)
{
    if(!nrfx_gpiote_is_init())
    {
        nrfx_gpiote_init();
    }
    
    nrfx_err_t err_code1 = nrfx_gpiote_in_init(BUTTON_1, &conf_toggle, db_on_toggle);
    // Configure board. 

    NRFX_ASSERT(on_press != NULL)
    db_event_user_on_press = on_press;
    NRFX_ASSERT(on_release != NULL)
    db_event_user_on_release = on_release;

    nrfx_gpiote_in_event_enable(BUTTON_1, true);

    return err_code1;
}

