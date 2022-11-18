
#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "math.h"

#include "nrf_delay.h"
#include "nrf_drv_clock.h"

#include "app_error.h"
#include "app_timer.h"

#include "blink_hal.h"
#include "button_2events.h"
#include "pwm_led.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"


#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 // ms

#define SLEEP_FRACTION_MS 10

#define PI (float) 3.14159265359
#define HOLD_MARGIN 500 // ms

// Blinks error state infinitely
void blink_error()
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

void id_to_led_blink(int num, int *dest, int size)
{
    int i;
    for(i = size - 1; i >= 0 ; i--)
    {
        dest[i] = num % 10;
        num /= 10;
    }
}

void (*custom_blink)(uint32_t led, uint32_t period, uint32_t num_periods);
volatile int blink_hold;
volatile int period_cur;
volatile int period_hold;

void custom_blink_wrapper(uint32_t led, uint32_t period, uint32_t num_periods)
{
    if(!period_hold)
    {
        period_cur = period;
    }

    do
    {
        custom_blink(led, period, num_periods);
    } while (blink_hold);
}

void smooth_blink(uint32_t led, uint32_t period, uint32_t num_periods)
{
    float value = ((float) period_cur / num_periods) * 2 * PI;
    pwm_led_value(led, sinf(value));
}

void discrete_blink(uint32_t led, uint32_t period, uint32_t num_periods)
{
    uint32_t ms = period * PWM_PERIOD_MS;
    float value = ((float) period_cur / num_periods) * 2 * PI;
    value = (ms < BLINK_DURATION) ? sinf(value) : 0;
    pwm_led_value(led, value);
}

APP_TIMER_DEF(double_click_timer);
volatile int num_press;

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
        NRF_LOG_INFO("Double click happened!!!!")
        app_timer_stop(double_click_timer);
        num_press = 0;
        blink_hold ^= 1;
    }
}

void logs_init()
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    
    if(ret != NRFX_SUCCESS)
    {
        blink_error();
    }

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void)
{
    led_init_all();
    pwm_led_init();
    logs_init();

    NRF_LOG_INFO("Starting up the test project with USB logging");

    NRF_LOG_INFO("Double event Initializing");
    nrfx_err_t error = db_event_init(BUTTON_1, on_press, on_release);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Double event Initialization finished");
    
    NRF_LOG_INFO("Timer creation");
    error = app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, double_click_timer_timeout);
    APP_ERROR_CHECK(error);
    NRF_LOG_INFO("Timer created");

    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, timing, LEDS_NUMBER);
    custom_blink = discrete_blink;

    NRF_LOG_INFO("Starting to blink ...");
    uint32_t num_periods = ROUNDED_DIV(BLINK_DURATION + PAUSE_DURATION, PWM_PERIOD_MS);
    blink_hold = 0;
    period_hold = 1;
    period_cur = num_periods / 4;

    while(true)
    {
        
        for(int led = 0; led < LEDS_NUMBER; led++)
        {
            for(int led_blinks = 0; led_blinks < timing[led]; led_blinks++)
            {
                NRF_LOG_INFO("Blinking led %d, %d / %d", led, led_blinks + 1, timing[led]);
                for(uint32_t period = 0; period < num_periods; period++)
                {
                    custom_blink_wrapper(led, period, num_periods);

                    LOG_BACKEND_USB_PROCESS();
                    NRF_LOG_PROCESS();
                }
            }
        }
        
    }
    
}


/**
 *@}
 **/
