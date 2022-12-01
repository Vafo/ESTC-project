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

static const nrfx_pwm_t pwm_instance = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_config_t pwm_config = {                                     \
    .output_pins  = { LED_1_GPIO,                                           \
                      LED_2R_GPIO,                                          \
                      LED_2G_GPIO,                                          \
                      LED_2B_GPIO },                                        \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
    .base_clock   = (nrf_pwm_clk_t)MAIN_PWM_BASE_CLOCK,                     \
    .count_mode   = (nrf_pwm_mode_t)NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,     \
    .top_value    = MAIN_PWM_TOP_VALUE,                                     \
    .load_mode    = (nrf_pwm_dec_load_t)NRF_PWM_LOAD_INDIVIDUAL,            \
    .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE,  \
};

static union
{
    nrf_pwm_values_individual_t spec;
    uint16_t raw[4];
} duty_cycles;

static uint16_t _proxy_buf[4];
static uint8_t _ready_to_copy;

static nrf_pwm_sequence_t pwm_seq = {
    .values = {
        .p_individual = &duty_cycles.spec
    },
    .length = 4,
    .repeats = 0,
    .end_delay = 10
};


uint8_t func_hold;

#define TOT_PERIODS ((uint32_t) (MAIN_PWM_FNC_DURATION / MAIN_PWM_PERIOD_MS))
#define SINE_AMPLITUDE 1
static uint32_t period_num;

void pwm_handler_half_sine(nrfx_pwm_evt_type_t event)
{
    float new_val;
    float time_var;
    switch (event)
    {
    case NRFX_PWM_EVT_END_SEQ0:
    case NRFX_PWM_EVT_END_SEQ1:
        if(!func_hold)
        {
            time_var = ((float) period_num / TOT_PERIODS) * PI;
            new_val = sinf(time_var);
            new_val = (new_val / SINE_AMPLITUDE) * MAIN_PWM_TOP_VALUE;
            NRF_LOG_INFO("Setting value %d PERIOD %d / %d", (uint16_t) new_val, period_num, TOT_PERIODS);
            for(int i = 0; i < 4; i++)
            {
                duty_cycles.raw[i] = (uint16_t) new_val;
            }

            if(++period_num >= TOT_PERIODS)
            {
                period_num = 0;
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
    nrfx_pwm_init(&pwm_instance, &pwm_config, pwm_handler_half_sine);
    nrfx_pwm_simple_playback(&pwm_instance, &pwm_seq, 1, NRFX_PWM_FLAG_LOOP | NRFX_PWM_FLAG_SIGNAL_END_SEQ0 | NRFX_PWM_FLAG_SIGNAL_END_SEQ1);
}


void pwm_led_value(uint32_t pin, float duty_cycle)
{
    pwm_led_individual_value_and_wait(pin, duty_cycle);
}

void pwm_led_individual_value(uint32_t led_idx, float duty_cycle)
{
    if(duty_cycle > 1)
    {
        duty_cycle = 1;
    } 
    else if(duty_cycle < 0)
    {
        duty_cycle = 0;
    }
    
    for(int i = 0; i < ARRAY_SIZE(duty_cycles.raw); i++)
    {
        _proxy_buf[i] = duty_cycles.raw[i];
    }

    _proxy_buf[led_idx] = duty_cycle * pwm_config.top_value;
    _ready_to_copy = 1;
    NRF_LOG_INFO("Set led %d to %d", led_idx, duty_cycle * pwm_config.top_value);
}

void pwm_led_individual_value_and_wait(uint32_t led_idx, float duty_cycle)
{
    pwm_led_individual_value(led_idx, duty_cycle);
    __WFE();
}