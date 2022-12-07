#include "main_logs.h"
#include "blink_hal.h"
#include "nrf_delay.h"

void blink_error()
{
    led_init(LED_2RED_IDX);
    while(1)
    {
        led_on(LED_2RED_IDX);
        nrf_delay_ms(250);
        led_off(LED_2RED_IDX);
        nrf_delay_ms(250);
    }
}

void main_logs_init()
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    
    if(ret != NRFX_SUCCESS)
    {
        blink_error();
    }

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}
