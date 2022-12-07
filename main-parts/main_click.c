#include "main_click.h"
#include "main_logs.h"
#include "main_pwm.h"

static main_pwm_math_fn funcs[] = {
    stair_func,
    sine_arc_func,
    saw_func
};

static void circular_increment(int *val, int top)
{
    (*val)++;
    if( *val >= top || *val < 0 )
    {
        *val = 0;
    }
}

static int led_fn_idx;
static int rgb_fn_idx;

void main_click_init()
{
    NRF_LOG_INFO("Double event Initializing");
    nrfx_err_t error = db_event_init(BUTTON_1, on_press, on_release, on_double_click);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Double event Initialization finished");

    led_fn_idx = 0;
    rgb_fn_idx = 1;
}

void on_press()
{
    func_hold = 1;
}

void on_release()
{
    func_hold = 0;
}

void on_double_click()
{
    NRF_LOG_INFO("Double click happened!!!!");

    circular_increment(&rgb_fn_idx, ARRAY_SIZE(funcs));
    circular_increment(&led_fn_idx, ARRAY_SIZE(funcs));
    NRF_LOG_INFO("LED %d RGB %d", led_fn_idx, rgb_fn_idx);
    rgb_math_fn = funcs[rgb_fn_idx];
    led_math_fn = funcs[led_fn_idx];

}