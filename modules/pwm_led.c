#include "pwm_led.h"
#include "nrfx_systick.h"
#include "blink_hal.h"
#include "nrfx_pwm.h"

#include "nrf52840.h"

#include "nrf_log.h"


#include "nrf_gpio.h"
#define LED_1_GPIO       NRF_GPIO_PIN_MAP(0,6)
#define LED_2R_GPIO      NRF_GPIO_PIN_MAP(0,8)
#define LED_2G_GPIO      NRF_GPIO_PIN_MAP(1,9)
#define LED_2B_GPIO      NRF_GPIO_PIN_MAP(0,12)

// Remove systick

// static nrfx_systick_state_t cur_state;
static const nrfx_pwm_t pwm_instance = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_config_t pwm_config = {                                    \
    .output_pins  = { LED_1_GPIO,                                          \
                      LED_2R_GPIO,                                         \
                      LED_2G_GPIO,                                         \
                      LED_2B_GPIO },                                       \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                  \
    .base_clock   = (nrf_pwm_clk_t)NRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK,     \
    .count_mode   = (nrf_pwm_mode_t)NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,    \
    .top_value    = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE,                     \
    .load_mode    = (nrf_pwm_dec_load_t)NRF_PWM_LOAD_INDIVIDUAL,           \
    .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE, \
};

union
{
    nrf_pwm_values_individual_t spec;
    uint16_t raw[4];
} duty_cycles;

// static uint16_t buf_cycles[4];

static nrf_pwm_sequence_t pwm_seq = {
    .values = {
        .p_individual = &duty_cycles.spec
    },
    .length = 4,
    .repeats = 5,
    .end_delay = 0
};

static uint16_t cnt;

void jalap_handler(nrfx_pwm_evt_type_t event)
{
    switch (event)
    {
    case NRFX_PWM_EVT_END_SEQ0:
    case NRFX_PWM_EVT_END_SEQ1:
        NRF_LOG_INFO("Copying to duty_cycles");
        for(int i = 0; i < 4; i++)
        {
            duty_cycles.raw[i] = cnt++;
        }
        if(cnt > 1000)
        {
            cnt = 0;
        }
        break;
    
    default:
        NRF_LOG_INFO("Unhandled event %d", event);
        break;
    }
}

void pwm_led_init(void)
{
    nrfx_pwm_init(&pwm_instance, &pwm_config, jalap_handler);
    nrfx_pwm_simple_playback(&pwm_instance, &pwm_seq, 5000, NRFX_PWM_FLAG_LOOP | NRFX_PWM_FLAG_SIGNAL_END_SEQ0 | NRFX_PWM_FLAG_SIGNAL_END_SEQ1);
}


void pwm_led_value(uint32_t led_idx, float duty_cycle)
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
        duty_cycles.raw[i] = 0;
    }
    duty_cycles.raw[led_idx] = duty_cycle * pwm_config.top_value;
    NRF_LOG_INFO("Set led %d to %d", led_idx, duty_cycle * pwm_config.top_value);
    // uint32_t active_duty = (uint32_t) (duty_cycle * PWM_PERIOD_US);
    // uint32_t inactive_duty = PWM_PERIOD_US - active_duty;
    // nrfx_systick_get(&cur_state);
    // led_on(led_idx);
    // while(!nrfx_systick_test(&cur_state, active_duty))
    //     ;
    // nrfx_systick_get(&cur_state);
    // led_off(led_idx);
    // while(!nrfx_systick_test(&cur_state, inactive_duty))
    //     ;

}