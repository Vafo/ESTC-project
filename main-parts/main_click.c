#include "main_click.h"
#include "main_blinking.h"
#include "main_logs.h"


void main_click_init()
{
    NRF_LOG_INFO("Double event Initializing");
    nrfx_err_t error = db_event_init(BUTTON_1, on_press, on_release, on_double_click);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Double event Initialization finished");
}

void on_press()
{
    intensity_hold = 0;
    NRF_LOG_INFO("Press happened!!!!");
    custom_blink_f = vary_intensity;
}

void on_release()
{
    NRF_LOG_INFO("Release happened!!!!");
    custom_blink_f = discrete_blink;
    intensity_hold = 1;
}

void on_double_click()
{
    NRF_LOG_INFO("Double click happened!!!!");
    blink_hold ^= 1;
}