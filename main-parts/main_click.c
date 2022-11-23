#include "main_click.h"
#include "main_blinking.h"
#include "main_logs.h"

#define HOLD_MARGIN 500 // ms

APP_TIMER_DEF(double_click_timer);
static volatile int num_press;


void main_click_init()
{
    NRF_LOG_INFO("Double event Initializing");
    nrfx_err_t error = db_event_init(BUTTON_1, on_press, on_release);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Double event Initialization finished");
    
    NRF_LOG_INFO("Timer creation");
    error = app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, double_click_timer_timeout);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Timer created");
}

void double_click_timer_timeout()
{
    num_press = 0;
}

void on_press()
{
    custom_blink = smooth_blink;
    period_hold = 0;
    if(num_press == 0)
    {
        app_timer_start(double_click_timer, APP_TIMER_TICKS(HOLD_MARGIN), NULL);
    }
    num_press++;
}

void on_release()
{
    custom_blink = discrete_blink;
    period_hold = 1;
    if(num_press == 2)
    {
        NRF_LOG_INFO("Double click happened!!!!");
        app_timer_stop(double_click_timer);
        num_press = 0;
        blink_hold ^= 1;
    }
}