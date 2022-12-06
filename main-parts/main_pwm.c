#include "main_pwm.h"
#include "blink_hal.h"
#include "nrf52840.h"
#include "nrf_log.h"
#include "nrf_gpio.h"

#include "pwm_abs.h"

#include "math.h"
#define ABS(X) ( (X) < 0 ? (-(X)) : (X))

#define LED_1_GPIO       NRF_GPIO_PIN_MAP(0,6)
#define LED_2R_GPIO      NRF_GPIO_PIN_MAP(0,8)
#define LED_2G_GPIO      NRF_GPIO_PIN_MAP(1,9)
#define LED_2B_GPIO      NRF_GPIO_PIN_MAP(0,12)

#define PI ((float) 3.14159265359)
#define HALF_PI (PI/2)

uint8_t func_hold;

#define SINE_AMPLITUDE 1

// Input [0 - 1]
// Output [0 - MAIN_PWM_TOP_VALUE]
float sine_arc_func(float input)
{
    float res;
    input = input * PI;
    res = sinf(input);
    res = (res / SINE_AMPLITUDE) * MAIN_PWM_TOP_VALUE;

    return res;
}

// Input [0 - 1]
// Output [0 - top_value]
float saw_func(float input, uint32_t top_value)
{
    float res;
    float inner_ = input - 0.5; 
    inner_ = 2*top_value * inner_;
    res = (-1) * ABS( inner_ ) + top_value;
    
    return res;
}

// Input [0 - 1]
// Output [0 - top_value]
float stair_func(float input, uint32_t top_value)
{
    float res = 0;
    if(input < 0.5)
    {
        res = top_value;
    }

    return res;
}

void pwm_handler_half_sine(nrfx_pwm_evt_type_t event_type, pwm_abs_op_cnxt_t *operational_context, uint32_t top_value)
{
    float new_val;
    float time_var;

    uint32_t *period_num_ptr = &operational_context->period_num;
    uint16_t *channels_ptr = (uint16_t *) &operational_context->channels;
    uint32_t tot_period = operational_context->tot_period;

    switch (event_type)
    {
    case NRFX_PWM_EVT_END_SEQ0:
    case NRFX_PWM_EVT_END_SEQ1:
        if(func_hold)
        {
            time_var = ((float) *period_num_ptr / tot_period);
            new_val = saw_func(time_var, top_value);
            NRF_LOG_INFO("Setting value %d PERIOD %d / %d", (uint16_t) new_val, *period_num_ptr, tot_period);
            for(int i = 0; i < 3; i++)
            {
                channels_ptr[i] = (uint16_t) new_val;
            }

            if(++(*period_num_ptr) >= tot_period)
            {
                *period_num_ptr = 0;
            }

        }
        break;
    
    default:
        // NRF_LOG_INFO("Unhandled event %d", event);
        break;
    }
}


void solo_handler(nrfx_pwm_evt_type_t event_type, pwm_abs_op_cnxt_t *operational_context, uint32_t top_value)
{
    float new_val;
    float time_var;

    uint32_t *period_num_ptr = &operational_context->period_num;
    uint16_t *channels_ptr = (uint16_t *) &operational_context->channels;
    uint32_t tot_period = operational_context->tot_period;

    switch (event_type)
    {
    case NRFX_PWM_EVT_END_SEQ0:
    case NRFX_PWM_EVT_END_SEQ1:
        // if(func_hold)
        {
            time_var = ((float) *period_num_ptr / tot_period);
            new_val = stair_func(time_var, top_value);
            NRF_LOG_INFO("Setting value %d PERIOD %d / %d", (uint16_t) new_val, *period_num_ptr, tot_period);
            channels_ptr[0] = (uint16_t) new_val;

            if(++(*period_num_ptr) >= tot_period)
            {
                *period_num_ptr = 0;
            }

        }
        break;
    
    default:
        // NRF_LOG_INFO("Unhandled event %d", event);
        break;
    }
}

static nrfx_pwm_t pwm_instance_rgb = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_config_t pwm_config_rgb = {                                 \
    .output_pins  = { LED_2R_GPIO,                                          \
                      LED_2G_GPIO,                                          \
                      LED_2B_GPIO,                                          \
                      NRFX_PWM_PIN_NOT_USED },                              \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
    .base_clock   = (nrf_pwm_clk_t)MAIN_PWM_BASE_CLOCK,                     \
    .count_mode   = (nrf_pwm_mode_t)NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,     \
    .top_value    = MAIN_PWM_TOP_VALUE,                                     \
    .load_mode    = (nrf_pwm_dec_load_t)NRF_PWM_LOAD_INDIVIDUAL,            \
    .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE,  \
};

static pwm_abs_cnxt_t rgb_cnxt = {
    .instance = &pwm_instance_rgb,
    .config = &pwm_config_rgb,
    .handler = pwm_handler_half_sine,
    .time_ms = 5000,
};


static nrfx_pwm_t pwm_instance_led0 = NRFX_PWM_INSTANCE(1);
static nrfx_pwm_config_t pwm_config_led0 = {                                \
    .output_pins  = { LED_1_GPIO,                                           \
                      NRFX_PWM_PIN_NOT_USED,                                \
                      NRFX_PWM_PIN_NOT_USED,                                \
                      NRFX_PWM_PIN_NOT_USED },                              \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
    .base_clock   = (nrf_pwm_clk_t)MAIN_PWM_BASE_CLOCK,                     \
    .count_mode   = (nrf_pwm_mode_t)NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,     \
    .top_value    = MAIN_PWM_TOP_VALUE,                                     \
    .load_mode    = (nrf_pwm_dec_load_t)NRF_PWM_LOAD_COMMON,                \
    .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE,  \
};

static pwm_abs_cnxt_t led_cnxt = {
    .instance = &pwm_instance_led0,
    .config = &pwm_config_led0,
    .handler = solo_handler,
    .time_ms = 1000,
};

// use user sequence

void pwm_led_init(void)
{
    pwm_abs_init(&rgb_cnxt);
    // pwm_abs_init(&led_cnxt);
}


void pwm_led_value(uint32_t pin, float duty_cycle)
{

}