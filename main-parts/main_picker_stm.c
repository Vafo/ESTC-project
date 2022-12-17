#include "main_picker_stm.h"
#include "main_logs.h"

#include "blink_hal.h"

#define LAST_2_DIGITS 50

static main_pwm_math_fn led_funcs[] = {
    off_func,
    stair_func,
    sine_arc_func,
    on_func    // change to on_func
};

main_pwm_math_fn led_math_fn;


picker_stm_cnxt_t stm_inst;
// Delete useless (dead) comments
// uint32_t set_period_to;

static void pwm_handler_rgb(nrfx_pwm_evt_type_t event_type, pwm_abs_op_cnxt_t *operational_context, uint32_t top_value)
{
    float time_var;
    float new_val = 0;

    uint32_t *period_num_ptr = &operational_context->period_num;
    nrf_pwm_values_wave_form_t *values_wave = (nrf_pwm_values_wave_form_t *) operational_context->values.p_wave_form;
    uint32_t tot_period = operational_context->tot_period;


    uint16_t r, g, b;

    // if(!stm_inst.updated)
    // {
    //     *period_num_ptr = set_period_to;
    // }

    switch (event_type)
    {
        case NRFX_PWM_EVT_END_SEQ0:
        case NRFX_PWM_EVT_END_SEQ1:
            if(func_hold)
            {
                time_var = ((float) *period_num_ptr / tot_period);
                new_val = saw_func(time_var, 1);
                hsv_update_component(&(stm_inst.hsv), stm_inst.cur_component, new_val);

                if(++(*period_num_ptr) >= tot_period)
                {
                    *period_num_ptr = 0;
                }
            }
            
            
            hsv_to_rgb(&(stm_inst.hsv), &stm_inst.rgb);
            
            r = values_wave->channel_0 = stm_inst.rgb.r * top_value;
            g = values_wave->channel_1 = stm_inst.rgb.g * top_value;
            b = values_wave->channel_2 = stm_inst.rgb.b * top_value;

            if(func_hold)
            {
                NRF_LOG_INFO("R : %d | G : %d | B : %d | new_val " NRF_LOG_FLOAT_MARKER, r, g, b, NRF_LOG_FLOAT(new_val));
            }

            
            break;
        
        default:
            // NRF_LOG_INFO("Unhandled event %d", event);
            break;
    }

}

static void pwm_handler_led(nrfx_pwm_evt_type_t event_type, pwm_abs_op_cnxt_t *operational_context, uint32_t top_value)
{
    float new_val;
    float time_var;

    uint32_t *period_num_ptr = &operational_context->period_num;
    uint16_t *channels_ptr = (uint16_t *) operational_context->values.p_common;
    uint32_t tot_period = operational_context->tot_period;
    // Rename to total_period

    switch (event_type)
    {
        case NRFX_PWM_EVT_END_SEQ0:
        case NRFX_PWM_EVT_END_SEQ1:
            // if(func_hold)
            {
                time_var = ((float) *period_num_ptr / tot_period);
                new_val = led_math_fn(time_var, top_value);
                // NRF_LOG_INFO("Setting value %d PERIOD %d / %d", (uint16_t) new_val, *period_num_ptr, tot_period);
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

static void circular_increment(picker_stm_mode_t *val, picker_stm_mode_t top)
{
    (*val)++;
    if( *val >= top || *val < 0 )
    {
        *val = 0;
    }
}

static void picker_state_exec(picker_stm_mode_t mode)
{
    float cur_val = 0;
    switch (mode)
    {
        case DISPLAY_MODE:
            stm_inst.cur_component = -1;
            cur_val = 0;
            break;

        case HUE_MODIFICATION_MODE:
            stm_inst.cur_component = HSV_H_IDX;
            cur_val = stm_inst.hsv.h;
            break;
            
        case SATURATION_MODIFICATION_MODE:
            stm_inst.cur_component = HSV_S_IDX;
            cur_val = stm_inst.hsv.s;
            break;
            
        case BRIGHTNESS_MODIFICATION_MODE:
            stm_inst.cur_component = HSV_V_IDX;
            cur_val = stm_inst.hsv.v;
            break;

        case PICKER_MODES_NUMBER:
            // Never to happen
            cur_val = 0;
            NRF_LOG_WARNING("IT HAPPEND?");
            break;
    }
    NRF_LOG_INFO("OUT OF STATE EXEC");
    (void)cur_val;
    // pwm_abs_cnxt_t *rgb_cnxt = stm_inst.rgb_cnxt;
    // pwm_abs_op_cnxt_t *op_cnxt = &(rgb_cnxt->op_cnxt);
    // set_period_to = cur_val * (op_cnxt->tot_period);
    // // stm_inst.updated = 0;
    
    led_math_fn = led_funcs[mode];
}   


void picker_stm_init()
{
    stm_inst.cur_mode = DISPLAY_MODE;
    float last_2_digits = ((float) LAST_2_DIGITS) / 100;
    hsv_set_values(&stm_inst.hsv, last_2_digits, 1, 1);

    func_hold = 0;

    stm_inst.led_cnxt = &led_cnxt;
    stm_inst.rgb_cnxt = &rgb_cnxt;

    main_pwm_init(pwm_handler_rgb, pwm_handler_led);
    picker_state_exec(DISPLAY_MODE);
}


void picker_stm_next_state() {
    circular_increment(&stm_inst.cur_mode, PICKER_MODES_NUMBER);
    picker_state_exec(stm_inst.cur_mode);

    NRF_LOG_INFO("CHANGED STATE TO %d", stm_inst.cur_mode);
}

void picker_stm_set_state(picker_stm_mode_t new_mode)
{
    ASSERT(new_mode >= 0 && new_mode < PICKER_MODES_NUMBER);
    stm_inst.cur_mode = new_mode;
    picker_state_exec(new_mode);
}

void picker_stm_on_press()
{
    if(stm_inst.cur_mode != DISPLAY_MODE)
    {
        func_hold = 1;
    }
}

void picker_stm_on_release()
{

    if(stm_inst.cur_mode != DISPLAY_MODE)
    {
        func_hold = 0;
    }
}

void picker_stm_on_double_click()
{
    picker_stm_next_state();
}