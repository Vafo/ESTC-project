#include "pwm_abs.h"

static pwm_abs_cnxt_t *instances[NRFX_PWM_ENABLED_COUNT];

#define PWM_ABS_HANDLER_NAME(PWM_IDX) NRFX_CONCAT_2(pwm_abs_handler_inst, PWM_IDX)

#define PWM_ABS_HANDLER_DEFINE(PWM_IDX)                                         \
void PWM_ABS_HANDLER_NAME(PWM_IDX) (nrfx_pwm_evt_type_t event_type)             \
{                                                                               \
    pwm_abs_update_handler handler = instances[(PWM_IDX)]->handler;             \
    uint32_t top_value = instances[(PWM_IDX)]->config->top_value;               \
    handler(event_type, &instances[(PWM_IDX)]->op_cnxt, top_value);                      \
}

// Can not directly paste NRFX_PWM0_INST_IDX, since it is enum
#if NRFX_CHECK(NRFX_PWM0_ENABLED)
    PWM_ABS_HANDLER_DEFINE(0)
#endif
#if NRFX_CHECK(NRFX_PWM1_ENABLED)
    PWM_ABS_HANDLER_DEFINE(1)
#endif
#if NRFX_CHECK(NRFX_PWM2_ENABLED)
    PWM_ABS_HANDLER_DEFINE(2)
#endif
#if NRFX_CHECK(NRFX_PWM3_ENABLED)
    PWM_ABS_HANDLER_DEFINE(3)
#endif


static nrfx_pwm_handler_t pwm_abs_inst_handlers[NRFX_PWM_ENABLED_COUNT] = {
    #if NRFX_CHECK(NRFX_PWM0_ENABLED)
        PWM_ABS_HANDLER_NAME(0),
    #endif
    #if NRFX_CHECK(NRFX_PWM1_ENABLED)
        PWM_ABS_HANDLER_NAME(1),
    #endif
    #if NRFX_CHECK(NRFX_PWM2_ENABLED)
        PWM_ABS_HANDLER_NAME(2),
    #endif
    #if NRFX_CHECK(NRFX_PWM3_ENABLED)
        PWM_ABS_HANDLER_NAME(3),
    #endif
};


static void _init_op_cnxt(pwm_abs_cnxt_t *cnxt)
{
    pwm_abs_op_cnxt_t *op_cnxt_ptr = &cnxt->op_cnxt;
    nrfx_pwm_config_t *cfg_ptr = cnxt->config;
    nrf_pwm_sequence_t *seq_ptr = cnxt->seq;

    op_cnxt_ptr->length = seq_ptr->length;
    op_cnxt_ptr->values = seq_ptr->values;

    uint16_t val_length = op_cnxt_ptr->length;
    uint16_t * val_raw = (uint16_t *) (op_cnxt_ptr->values.p_raw);
    for (int i = 0; i < val_length; i++)
    {
        val_raw[i] = 0;
    }

    op_cnxt_ptr->period_num = 0;
    uint32_t freq = PWM_ABS_IDX_TO_FREQ(cfg_ptr->base_clock);
    float period_ms = PWM_ABS_FREQ_TO_PERIOD_MS(freq) * cfg_ptr->top_value;
    op_cnxt_ptr->tot_period = (cnxt->time_ms / period_ms) * 2;
    
    uint32_t top_val = cfg_ptr->top_value;
    if(cfg_ptr->load_mode == NRF_PWM_LOAD_WAVE_FORM)
    {
        for(int top_val_idx = 3; top_val_idx < val_length; top_val_idx += 4)
        {
            val_raw[top_val_idx] = top_val;
        }
    }
}

nrfx_err_t pwm_abs_init(pwm_abs_cnxt_t *cnxt)
{
    nrfx_pwm_t *inst_ptr = cnxt->instance;
    nrfx_pwm_config_t *cfg_ptr = cnxt->config;
    nrf_pwm_sequence_t *seq_ptr = cnxt->seq;
    uint8_t inst_idx = inst_ptr->drv_inst_idx;

    NRFX_ASSERT(inst_idx >= 0 && inst_idx < NRFX_PWM_ENABLED_COUNT);

    nrfx_pwm_uninit(inst_ptr);
    nrfx_pwm_init(inst_ptr, cfg_ptr, pwm_abs_inst_handlers[inst_idx]);

    _init_op_cnxt(cnxt);

    instances[inst_idx] = cnxt;
    nrfx_pwm_simple_playback(inst_ptr, seq_ptr, 1, NRFX_PWM_FLAG_LOOP | NRFX_PWM_FLAG_SIGNAL_END_SEQ0 | NRFX_PWM_FLAG_SIGNAL_END_SEQ1);

    return NRFX_SUCCESS;
}


void pwm_abs_update_func(pwm_abs_cnxt_t *cnxt, pwm_abs_update_handler new_handler)
{
    
    cnxt->handler = new_handler;

    _init_op_cnxt(cnxt);

}