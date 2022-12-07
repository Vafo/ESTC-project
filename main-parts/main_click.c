#include "main_click.h"
#include "main_logs.h"
#include "main_picker_stm.h"

void main_click_init()
{
    NRF_LOG_INFO("Double event Initializing");
    nrfx_err_t error = db_event_init(BUTTON_1, on_press, on_release, on_double_click);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Double event Initialization finished");
}

void on_press()
{
    picker_stm_on_press();
}

void on_release()
{
    picker_stm_on_release();
}

void on_double_click()
{
    NRF_LOG_INFO("Double click happened!!!!");

    picker_stm_on_double_click();
}