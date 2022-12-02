#include "main_pwm.h"
#include "blink_hal.h"
#include "nrf52840.h"
#include "nrf_log.h"
#include "nrf_gpio.h"

#include "nrfx_pwm.h"

#include "math.h"
#define ABS(X) ( (X) < 0 ? (-(X)) : (X))

#define LED_1_GPIO       NRF_GPIO_PIN_MAP(0,6)
#define LED_2R_GPIO      NRF_GPIO_PIN_MAP(0,8)
#define LED_2G_GPIO      NRF_GPIO_PIN_MAP(1,9)
#define LED_2B_GPIO      NRF_GPIO_PIN_MAP(0,12)

#define PI ((float) 3.14159265359)
#define HALF_PI (PI/2)

static union
{
    nrf_pwm_values_wave_form_t spec;
    uint16_t raw[4];
} duty_cycles_rgb;

static const nrfx_pwm_t pwm_instance_rgb = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_config_t pwm_config_rgb = {                                 \
    .output_pins  = { LED_2R_GPIO,                                          \
                      LED_2G_GPIO,                                          \
                      LED_2B_GPIO,                                          \
                      NRFX_PWM_PIN_NOT_USED },                              \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
    .base_clock   = (nrf_pwm_clk_t)MAIN_PWM_BASE_CLOCK,                     \
    .count_mode   = (nrf_pwm_mode_t)NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,     \
    .top_value    = MAIN_PWM_TOP_VALUE,                                     \
    .load_mode    = (nrf_pwm_dec_load_t)NRF_PWM_LOAD_WAVE_FORM,             \
    .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE,  \
};

static nrf_pwm_sequence_t pwm_seq_rgb = {
    .values = {
        .p_wave_form = &duty_cycles_rgb.spec
    },
    .length = 4,
    .repeats = 0,
    .end_delay = 0
};


nrf_pwm_values_common_t duty_cycles_led0;

static const nrfx_pwm_t pwm_instance_led0 = NRFX_PWM_INSTANCE(1);
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

static nrf_pwm_sequence_t pwm_seq_led0 = {
    .values = {
        .p_common = &duty_cycles_led0
    },
    .length = 1,
    .repeats = 0,
    .end_delay = 0
};

static uint8_t _ready_to_copy;



uint8_t func_hold;

#define TOT_PERIODS ((uint32_t) (MAIN_PWM_FNC_DURATION / MAIN_PWM_PERIOD_MS) * 2)
#define TOT_PERIODS_LED0 ((uint32_t) (1000 / MAIN_PWM_PERIOD_MS) * 2)
#define SINE_AMPLITUDE 1
static uint32_t period_num_rgb;
static uint32_t period_num_led0;

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
// Output [0 - MAIN_PWM_TOP_VALUE]
float saw_func(float input)
{
    float res;
    float inner_ = input - 0.5; 
    inner_ = 2*MAIN_PWM_TOP_VALUE * inner_;
    res = (-1) * ABS( inner_ ) + MAIN_PWM_TOP_VALUE;
    
    return res;
}

// Input [0 - 1]
// Output [0 - MAIN_PWM_TOP_VALUE]
float stair_func(float input)
{
    float res = 0;
    if(input < 0.5)
    {
        res = 1000;
    }

    return res;
}

void pwm_handler_half_sine(nrfx_pwm_evt_type_t event)
{
    float new_val;
    float time_var;
    switch (event)
    {
    case NRFX_PWM_EVT_END_SEQ0:
    case NRFX_PWM_EVT_END_SEQ1:
        if(func_hold)
        {
            time_var = ((float) period_num_rgb / TOT_PERIODS);
            new_val = saw_func(time_var);
            NRF_LOG_INFO("Setting value %d PERIOD %d / %d", (uint16_t) new_val, period_num_rgb, TOT_PERIODS);
            for(int i = 0; i < 3; i++)
            {
                duty_cycles_rgb.raw[i] = (uint16_t) new_val;
            }

            if(++period_num_rgb >= TOT_PERIODS)
            {
                period_num_rgb = 0;
            }

        }
        break;
    
    default:
        // NRF_LOG_INFO("Unhandled event %d", event);
        break;
    }
}

void solo_handler(nrfx_pwm_evt_type_t event)
{
    float new_val;
    float time_var;
    switch (event)
    {
    case NRFX_PWM_EVT_END_SEQ0:
    case NRFX_PWM_EVT_END_SEQ1:
        // if(func_hold)
        {
            time_var = ((float) period_num_led0 / TOT_PERIODS_LED0);
            new_val = stair_func(time_var);
            NRF_LOG_INFO("Setting value %d PERIOD %d / %d", (uint16_t) new_val, period_num_led0, TOT_PERIODS_LED0);
            duty_cycles_led0 = (uint16_t) new_val;

            if(++period_num_led0 >= TOT_PERIODS_LED0)
            {
                period_num_led0 = 0;
            }

        }
        break;
    
    default:
        // NRF_LOG_INFO("Unhandled event %d", event);
        break;
    }
}

// void pwm_handler(nrfx_pwm_evt_type_t event)
// {
//     switch (event)
//     {
//     case NRFX_PWM_EVT_END_SEQ0:
//     case NRFX_PWM_EVT_END_SEQ1:
//         if(_ready_to_copy)
//         {
//             NRF_LOG_INFO("Copying to duty_cycles");
//             for(int i = 0; i < 4; i++)
//             {
//                 duty_cycles.raw[i] = _proxy_buf[i];
//             }
//             __SEV();
//             _ready_to_copy = 0;
//         }
//         break;
    
//     default:
//         // NRF_LOG_INFO("Unhandled event %d", event);
//         break;
//     }
// }

void pwm_led_init(void)
{
    _ready_to_copy = 0;
    func_hold = 0;
    duty_cycles_rgb.spec.counter_top = MAIN_PWM_TOP_VALUE;
    nrfx_pwm_init(&pwm_instance_rgb, &pwm_config_rgb, pwm_handler_half_sine);
    nrfx_pwm_simple_playback(&pwm_instance_rgb, &pwm_seq_rgb, 1, NRFX_PWM_FLAG_LOOP | NRFX_PWM_FLAG_SIGNAL_END_SEQ0 | NRFX_PWM_FLAG_SIGNAL_END_SEQ1);

    nrfx_pwm_init(&pwm_instance_led0, &pwm_config_led0, solo_handler);
    nrfx_pwm_simple_playback(&pwm_instance_led0, &pwm_seq_led0, 1, NRFX_PWM_FLAG_LOOP | NRFX_PWM_FLAG_SIGNAL_END_SEQ0 | NRFX_PWM_FLAG_SIGNAL_END_SEQ1);
}


void pwm_led_value(uint32_t pin, float duty_cycle)
{

}