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

typedef struct 
{
    uint32_t pin;
    db_event_handler on_press[DB_EVENT_MAX_HANDLERS_PER_PIN];
    db_event_handler on_release[DB_EVENT_MAX_HANDLERS_PER_PIN];
    uint8_t valid;
    app_timer_id_t const *debouncer;
} db_event_pin_handlers_t;

static uint8_t db_event_pin_handlers_initialized = 0;
static db_event_pin_handlers_t pin_handlers[DB_EVENT_MAX_PINS];

static uint8_t db_event_pin_idx_to_place(uint32_t pin)
{
    int i, vacant = -1;
    int res = -1;
    for(i = 0; i < DB_EVENT_MAX_PINS; i++)
    {
        if(pin_handlers[i].valid && pin_handlers[i].pin == pin)
        {
            break;
        }
        if(!pin_handlers[i].valid)
        {
            vacant = i;
        }
    }

    if(i < DB_EVENT_MAX_PINS)
    {
        res = i;
    }
    else if(vacant >= 0 && vacant < DB_EVENT_MAX_PINS)
    {
        res = vacant;
    }

    return res;
}


APP_TIMER_DEF(debouncer_timer_1);
APP_TIMER_DEF(debouncer_timer_2);

#define TICKS_1MS APP_TIMER_TICKS(1)

static void lfclk_request()
{
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}

void debouncer_timeout_handler(void *p_context)
{
    db_event_pin_handlers_t *pin_handle = p_context;
    debug_led_green_off();
    if(nrf_gpio_pin_read(BUTTON_1) == 0)
    {
        debug_led_blue_on();
        for (int handler = 0; handler < DB_EVENT_MAX_HANDLERS_PER_PIN; handler++)
        {
            if(pin_handle->on_press[handler] != NULL)
            {
                pin_handle->on_press[handler]();
            }
        }
        
    }
    else
    {
        debug_led_blue_off();
        for (int handler = 0; handler < DB_EVENT_MAX_HANDLERS_PER_PIN; handler++)
        {
            if(pin_handle->on_release[handler] != NULL)
            {
                pin_handle->on_release[handler]();
            }
        }
    }
}

void db_on_toggle(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // debug_led_blue_off();
    debug_led_green_on();
    int idx = db_event_pin_idx_to_place(pin);
    if(idx < 0 || idx >= DB_EVENT_MAX_PINS)
    {
        db_blink_error();
    }

    db_event_pin_handlers_t *pin_handle = pin_handlers + idx;

    ret_code_t err;
    err = app_timer_stop(*(pin_handle->debouncer));
    if(err != NRFX_SUCCESS)
        db_blink_error();
    err = app_timer_start(*(pin_handle->debouncer), TICKS_1MS, pin_handle);
    if(err != NRFX_SUCCESS)
        db_blink_error();
}


nrfx_err_t db_event_init()
{
    // debug_led_blue_on();
    if(!nrfx_gpiote_is_init())
    {
        nrfx_gpiote_init();
    }
    
    lfclk_request();
    ret_code_t err_timer;
    err_timer = app_timer_init();
    if(err_timer != NRFX_SUCCESS)
    {
        return err_timer;
    }
    
    app_timer_id_t const *debouncers[DB_EVENT_MAX_PINS] = {&debouncer_timer_1, &debouncer_timer_2};
    if(!db_event_pin_handlers_initialized)
    {
        db_event_pin_handlers_initialized = 1;
        for(int i = 0; i < DB_EVENT_MAX_PINS; i++)
        {
            pin_handlers[i].valid = 0;
            pin_handlers[i].debouncer = debouncers[i];
            for(int j = 0; j < DB_EVENT_MAX_HANDLERS_PER_PIN; j++)
            {
                pin_handlers[i].on_press[j] = NULL;
                pin_handlers[i].on_release[j] = NULL;
            }
        }
    }

    return NRFX_SUCCESS;
}

nrfx_err_t db_event_add(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release)
{
    int idx = db_event_pin_idx_to_place(pin);
    int already_valid = 0;
    int hndlr_idx;
    if(idx == -1)
    {
        return NRFX_ERROR_NO_MEM;
    }

    already_valid = pin_handlers[idx].valid;

    if(!already_valid)
    {
        nrfx_err_t err_code_nrfx = nrfx_gpiote_in_init(pin, &conf_toggle, db_on_toggle);
        if(err_code_nrfx != NRFX_SUCCESS)
        {
            return err_code_nrfx;
        }
    }

    for(hndlr_idx = 0; hndlr_idx < DB_EVENT_MAX_HANDLERS_PER_PIN; hndlr_idx++)
    {
        if(pin_handlers[idx].on_press[hndlr_idx] == NULL && \
            pin_handlers[idx].on_release[hndlr_idx] == NULL)
        {
            break;   
        }
    }

    if(hndlr_idx < DB_EVENT_MAX_HANDLERS_PER_PIN)
    {
        NRFX_ASSERT(on_press != NULL)
        NRFX_ASSERT(on_release != NULL)
        pin_handlers[idx].on_press[hndlr_idx] = on_press;
        pin_handlers[idx].on_release[hndlr_idx] = on_release;
    }
    else
    {
        return NRFX_ERROR_NO_MEM;
    }

    pin_handlers[idx].pin = pin;
    pin_handlers[idx].valid = 1;

    nrfx_gpiote_in_event_enable(pin, true);

    if(!already_valid)
    {
        nrfx_err_t err_timer = app_timer_create(pin_handlers[idx].debouncer, APP_TIMER_MODE_SINGLE_SHOT, debouncer_timeout_handler);
        if(err_timer != NRFX_SUCCESS)
        {
            return err_timer;
        }
    }

    return NRFX_SUCCESS;
}


nrfx_err_t db_event_delete_pin(nrfx_gpiote_pin_t pin)
{
    int idx = db_event_pin_idx_to_place(pin);
    if(idx == -1)
    {
        return NRFX_ERROR_INVALID_PARAM;
    }

    db_event_pin_handlers_t *handles = pin_handlers + idx;

    if(!handles->valid)
    {
        return NRFX_SUCCESS;
    }

    handles->pin = -1;
    handles->valid = 0;
    for(int handler = 0; handler < DB_EVENT_MAX_HANDLERS_PER_PIN; handler++)
    {
        handles->on_press[handler] = NULL;
        handles->on_release[handler] = NULL;
    }

    app_timer_stop(*(handles->debouncer));
    nrfx_gpiote_in_uninit(pin);

    return NRFX_SUCCESS;
}

nrfx_err_t db_event_delete_handler(nrfx_gpiote_pin_t pin, db_event_handler on_press, db_event_handler on_release)
{
    int handler;
    int idx = db_event_pin_idx_to_place(pin);
    if(idx == -1)
    {
        return NRFX_ERROR_INVALID_PARAM;
    }

    db_event_pin_handlers_t *handles = pin_handlers + idx;

    if(!handles->valid)
    {
        return NRFX_SUCCESS;
    }

    for(handler = 0; handler < DB_EVENT_MAX_HANDLERS_PER_PIN; handler++)
    {
        if(handles->on_press[handler] == on_press && \
            handles->on_release[handler] == on_release)
        {
            break;   
        }
    }
    
    if(handler >= DB_EVENT_MAX_HANDLERS_PER_PIN)
    {
        return NRFX_ERROR_INVALID_PARAM;
    }

    handles->on_press[handler] = NULL;
    handles->on_release[handler] = NULL;

    return NRFX_SUCCESS;
}