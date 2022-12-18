#include "button_2events.h"
#include "button_hal.h"

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
void (*db_event_user_on_double_click)(void);

APP_TIMER_DEF(debouncer_timer);
#define TICKS_1MS APP_TIMER_TICKS(1)

// Double clicky part
APP_TIMER_DEF(double_click_timer);
static volatile uint8_t num_press;
static volatile uint8_t after_db_timeout;

static void lfclk_request()
{
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}

void debouncer_timeout_handler(void *p_context)
{
    debug_led_green_off();
    if(button_is_pressed(BUTTON_1))
    {
        debug_led_blue_on();

        if(num_press == 0)
        {
            after_db_timeout = 0;
            app_timer_start(double_click_timer, APP_TIMER_TICKS(DB_HOLD_MARGIN), NULL);
        }
        num_press++;
    }
    else
    {
        debug_led_blue_off();
        if(after_db_timeout)
        {
            db_event_user_on_release();
        }

        if(num_press == 2)
        {
            app_timer_stop(double_click_timer);
            num_press = 0;
            db_event_user_on_double_click();
        }
    }
}

void double_click_timeout_handler()
{
    if(num_press == 1 && button_is_pressed(BUTTON_1))
    {
        db_event_user_on_press();
        after_db_timeout = 1;
    }
    num_press = 0;
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


nrfx_err_t db_event_init(nrfx_gpiote_pin_t pin, 
                        db_event_handler on_press, 
                        db_event_handler on_release,
                        db_event_handler on_double_click)
{
    debug_led_blue_on();
    if(!nrfx_gpiote_is_init())
    {
        nrfx_gpiote_init();
    }

    nrfx_err_t err_code_nrfx = nrfx_gpiote_in_init(BUTTON_1, &conf_toggle, db_on_toggle);
    APP_ERROR_CHECK(err_code_nrfx);
    
    lfclk_request();
    ret_code_t err_timer;
    err_timer = app_timer_init();
    APP_ERROR_CHECK(err_timer);
    err_timer = app_timer_create(&debouncer_timer, APP_TIMER_MODE_SINGLE_SHOT, debouncer_timeout_handler);
    APP_ERROR_CHECK(err_timer);

    NRF_LOG_INFO("Double Button events: double click timer creation");
    err_timer = app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, double_click_timeout_handler);
    APP_ERROR_CHECK(err_timer);
    num_press = 0;
    after_db_timeout = 0;
    NRF_LOG_INFO("Double Button events: double click timer created");

    NRFX_ASSERT(on_press != NULL)
    db_event_user_on_press = on_press;
    NRFX_ASSERT(on_release != NULL)
    db_event_user_on_release = on_release;
    NRFX_ASSERT(on_double_click != NULL)
    db_event_user_on_double_click = on_double_click;
    
    nrfx_gpiote_in_event_enable(BUTTON_1, true);

    return NRFX_SUCCESS;
}

