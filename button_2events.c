#include "button_2events.h"
#include "app_timer.h"
#include "drv_rtc.h"
#include "nrf_drv_clock.h"
#include "nrfx_clock.h"

#define BLINK_DEBUG 0

#if BLINK_DEBUG == 1
#include "blink_hal.h"
#include "nrf_delay.h"
#define debug_led_green_on() led_on(LED_2GREEN_IDX)
#define debug_led_green_off() led_off(LED_2GREEN_IDX)
#define debug_led_blue_on() led_on(LED_2BLUE_IDX)
#define debug_led_blue_off() led_off(LED_2BLUE_IDX)

void db_blink_error()
{
    led_init(LED_2RED_IDX);
    while(true)
    {
        led_on(LED_2RED_IDX);
        nrf_delay_ms(250);
        led_off(LED_2RED_IDX);
        nrf_delay_ms(250);
    }
}

#else
#define debug_led_green_on()
#define debug_led_green_off()
#define debug_led_blue_on()
#define debug_led_blue_off()
#define db_blink_error()
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


/*
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
*/

APP_TIMER_DEF(debouncer_timer);
#define TICKS_1MS APP_TIMER_TICKS(1)

static void lfclk_request()
{
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}

void debouncer_timeout_handler(void *p_context)
{
    debug_led_green_off();
    if(nrf_gpio_pin_read(BUTTON_1) == 0)
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

void db_on_toggle(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    debug_led_blue_off();
    debug_led_green_on();
    
    ret_code_t err;
    err = app_timer_stop(debouncer_timer);
    if(err != NRFX_SUCCESS)
        db_blink_error();
    err = app_timer_start(debouncer_timer, TICKS_1MS, NULL);
    if(err != NRFX_SUCCESS)
        db_blink_error();
}


nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release)
{
    debug_led_blue_on();
    if(!nrfx_gpiote_is_init())
    {
        nrfx_gpiote_init();
    }

    nrfx_err_t err_code_nrfx = nrfx_gpiote_in_init(BUTTON_1, &conf_toggle, db_on_toggle);
    if(err_code_nrfx != NRFX_SUCCESS)
    {
        return err_code_nrfx;
    }
    
    lfclk_request();
    ret_code_t err_timer;
    err_timer = app_timer_init();
    if(err_timer != NRFX_SUCCESS)
    {
        return err_timer;
    }
    err_timer = app_timer_create(&debouncer_timer, APP_TIMER_MODE_SINGLE_SHOT, debouncer_timeout_handler);
    if(err_timer != NRFX_SUCCESS)
    {
        return err_timer;
    }

    NRFX_ASSERT(on_press != NULL)
    db_event_user_on_press = on_press;
    NRFX_ASSERT(on_release != NULL)
    db_event_user_on_release = on_release;

    nrfx_gpiote_in_event_enable(BUTTON_1, true);

    return NRFX_SUCCESS;
}

