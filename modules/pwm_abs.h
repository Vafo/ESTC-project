#ifndef PWM_ABS_H
#define PWM_ABS_H

#include <stdint.h>
#include "nrfx_pwm.h"
#include "app_util.h"


/** 
 * @brief PWM Base clock frequency: index to Hz. 
 *        Formula: 16Mhz / 2^idx
 * @param[in] idx index among
 * */
#define PWM_ABS_IDX_TO_FREQ(idx) ((16000000) >> (idx))
#define PWM_ABS_FREQ_TO_PERIOD_MS(freq) (((float) 1000) / (freq)) 

typedef struct 
{
    nrf_pwm_values_t values;
    uint16_t length;
    uint32_t period_num;
    uint32_t tot_period;
} pwm_abs_op_ctx_t;

typedef uint32_t pwm_abs_time_ms;

typedef void (*pwm_abs_update_handler)(nrfx_pwm_evt_type_t event_type, pwm_abs_op_ctx_t *operational_context, uint32_t top_value);

typedef struct 
{
    pwm_abs_op_ctx_t op_ctx;

    nrfx_pwm_t *instance;
    nrfx_pwm_config_t *config;
    nrf_pwm_sequence_t *seq;

    pwm_abs_update_handler handler;
    pwm_abs_time_ms time_ms;
} pwm_abs_ctx_t;


nrfx_err_t pwm_abs_init(pwm_abs_ctx_t *ctx);
void pwm_abs_change_func(pwm_abs_ctx_t *ctx, pwm_abs_update_handler new_handler);
void pwm_abs_set_period(pwm_abs_ctx_t *ctx, uint32_t period);
nrfx_err_t pwm_abs_uninit(pwm_abs_ctx_t *ctx);


#endif