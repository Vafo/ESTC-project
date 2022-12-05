#ifndef PWM_LED_H
#define PWM_LED_H

#include <stdint.h>
#include "nrfx_pwm.h"
#include "app_util.h"

/** @brief PWM Base clock index. */
#define MAIN_PWM_BASE_CLOCK 4

/** @brief PWM Top value. */
#define MAIN_PWM_TOP_VALUE 1000
/** @brief Function duration in ms*/
#define MAIN_PWM_FNC_DURATION 2500

/** 
 * @brief PWM Base clock frequency: index to Hz. 
 *        Formula: 16Mhz / 2^idx
 * @param[in] idx index among
 * */
#define PWM_ABS_IDX_TO_FREQ(idx) ((16000000) / (1 << (idx))) 
#define PWM_ABS_FREQ_TO_PERIOD_MS(freq) (((float) 1000) / (freq)) 

typedef struct 
{
    uint16_t channels[4];
    uint32_t period_num;
    uint32_t tot_period;
} pwm_abs_op_cnxt_t;


typedef void (*pwm_abs_update_handler)(nrfx_pwm_evt_type_t event_type, pwm_abs_op_cnxt_t *operational_context);

typedef uint32_t pwm_abs_time_ms;

typedef struct 
{
    pwm_abs_op_cnxt_t op_cnxt;

    nrfx_pwm_t *instance;
    nrfx_pwm_config_t *config;
    nrf_pwm_sequence_t seq;

    pwm_abs_update_handler handler;
    pwm_abs_time_ms time_ms;
} pwm_abs_cnxt_t;

nrfx_err_t pwm_abs_init(pwm_abs_cnxt_t *cnxt);
void pwm_abs_update_func(pwm_abs_cnxt_t *cnxt, pwm_abs_update_handler *new_handler);
nrfx_err_t pwm_abs_uninit(pwm_abs_cnxt_t *cnxt);


#endif