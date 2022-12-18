#include "main_click.h"
#include "main_logs.h"
#include "main_picker_fsm.h"

void main_click_init()
{
    NRF_LOG_INFO("Double event Initializing");
    nrfx_err_t error = db_event_init(BUTTON_1, on_press, on_release, on_double_click);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Double event Initialization finished");
}

void on_press()
{
    picker_fsm_press_handler();
}

void on_release()
{
    picker_fsm_release_handler();
}

void on_double_click()
{
    NRF_LOG_INFO("Double click happened!!!!");

    picker_fsm_double_click_handler();
}