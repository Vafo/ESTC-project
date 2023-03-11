#include "main_picker_fsm.h"
#include "main_logs.h"

#include "blink_hal.h"

#define LAST_2_DIGITS 50

static main_pwm_math_fn led_funcs[] = {
    off_func,
    stair_func,
    sine_arc_func,
    on_func
};

main_pwm_math_fn led_math_fn;


picker_fsm_ctx_t fsm_inst;

rgb damn;
// Delete useless (dead) comments

static void pwm_handler_rgb(nrfx_pwm_evt_type_t event_type, pwm_abs_op_ctx_t *operational_context, uint32_t top_value)
{
    float time_var;
    float new_val = 0;

    uint32_t *period_num_ptr = &operational_context->period_num;
    nrf_pwm_values_wave_form_t *values_wave = (nrf_pwm_values_wave_form_t *) operational_context->values.p_wave_form;
    uint32_t tot_period = operational_context->tot_period;


    uint16_t r, g, b;

    if(!fsm_inst.updated)
    {
        *period_num_ptr = fsm_inst.set_period_to * tot_period;
    }

    switch (event_type)
    {
        case NRFX_PWM_EVT_END_SEQ0:
        case NRFX_PWM_EVT_END_SEQ1:
            if(func_hold)
            {
                time_var = ((float) *period_num_ptr / tot_period);
                new_val = saw_func(time_var, 1);
                hsv_update_component(&(fsm_inst.hsv), fsm_inst.cur_component, new_val);

                if(++(*period_num_ptr) >= tot_period)
                {
                    *period_num_ptr = 0;
                }
                
                fsm_inst.updated = 0;
            }
            
            
            if(!fsm_inst.updated)
            {
                hsv_to_rgb(&(fsm_inst.hsv), &fsm_inst.rgb);
            
                r = values_wave->channel_0 = fsm_inst.rgb.r * top_value;
                g = values_wave->channel_1 = fsm_inst.rgb.g * top_value;
                b = values_wave->channel_2 = fsm_inst.rgb.b * top_value;
                fsm_inst.updated = 1;
            }

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

static void pwm_handler_led(nrfx_pwm_evt_type_t event_type, pwm_abs_op_ctx_t *operational_context, uint32_t top_value)
{
    float new_val;
    float time_var;

    uint32_t *period_num_ptr = &operational_context->period_num;
    uint16_t *channels_ptr = (uint16_t *) operational_context->values.p_common;
    uint32_t total_period = operational_context->tot_period;
    // Rename to total_period

    switch (event_type)
    {
        case NRFX_PWM_EVT_END_SEQ0:
        case NRFX_PWM_EVT_END_SEQ1:
            
            time_var = ((float) *period_num_ptr / total_period);
            new_val = led_math_fn(time_var, top_value);
            // NRF_LOG_INFO("Setting value %d PERIOD %d / %d", (uint16_t) new_val, *period_num_ptr, tot_period);
            channels_ptr[0] = (uint16_t) new_val;

            if(++(*period_num_ptr) >= total_period)
            {
                *period_num_ptr = 0;
            }

            break;
        
        default:
            // NRF_LOG_INFO("Unhandled event %d", event);
            break;
    }
}

static void circular_increment(picker_fsm_mode_t *val, picker_fsm_mode_t top)
{
    (*val)++;
    if( *val >= top || *val < 0 )
    {
        *val = 0;
    }
}

static void picker_state_exec(picker_fsm_mode_t mode)
{
    float cur_val = 0;
    switch (mode)
    {
        case DISPLAY_MODE:
            fsm_inst.cur_component = -1;
            cur_val = 0;
            break;

        case HUE_MODIFICATION_MODE:
            fsm_inst.cur_component = HSV_H_IDX;
            cur_val = fsm_inst.hsv.h;
            break;
            
        case SATURATION_MODIFICATION_MODE:
            fsm_inst.cur_component = HSV_S_IDX;
            cur_val = fsm_inst.hsv.s;
            break;
            
        case BRIGHTNESS_MODIFICATION_MODE:
            fsm_inst.cur_component = HSV_V_IDX;
            cur_val = fsm_inst.hsv.v;
            break;

        case PICKER_MODES_NUMBER:
            // Never to happen
            cur_val = 0;
            NRF_LOG_WARNING("IT HAPPEND?");
            break;
    }
    NRF_LOG_INFO("OUT OF STATE EXEC");
    // I can not update here fsm_inst.rgb_ctx->op_ctx.period_num directly
    // Why??
    cur_val /= 2;
    fsm_inst.updated = 0;
    fsm_inst.set_period_to = cur_val;

    led_math_fn = led_funcs[mode];
}   


void picker_fsm_init()
{
    fsm_inst.cur_mode = DISPLAY_MODE;
    float last_2_digits = ((float) LAST_2_DIGITS) / 100;
    hsv_set_values(&fsm_inst.hsv, last_2_digits, 1, 1);

    func_hold = 0;

    fsm_inst.led_ctx = &led_ctx;
    fsm_inst.rgb_ctx = &rgb_ctx;
    fsm_inst.set_period_to = 0;
    fsm_inst.updated = 0;

    main_pwm_init(pwm_handler_rgb, pwm_handler_led);
    picker_state_exec(DISPLAY_MODE);
    picker_fsm_get_hsv();
}


void picker_fsm_next_state() 
{
    picker_fsm_mode_t prev_mode = fsm_inst.cur_mode;
    circular_increment(&fsm_inst.cur_mode, PICKER_MODES_NUMBER);
    picker_state_exec(fsm_inst.cur_mode);

    if(prev_mode == BRIGHTNESS_MODIFICATION_MODE && fsm_inst.cur_mode == DISPLAY_MODE)
    {
        picker_fsm_save_hsv(&(fsm_inst.hsv));
        // picker_fsm_get_hsv();
    }

    NRF_LOG_INFO("CHANGED STATE TO %d", fsm_inst.cur_mode);
}

void picker_fsm_set_state(picker_fsm_mode_t new_mode)
{
    ASSERT(new_mode >= 0 && new_mode < PICKER_MODES_NUMBER);
    fsm_inst.cur_mode = new_mode;
    picker_state_exec(new_mode);
}

void picker_fsm_press_handler()
{
    if(fsm_inst.cur_mode != DISPLAY_MODE)
    {
        func_hold = 1;
    }
}

void picker_fsm_release_handler()
{

    if(fsm_inst.cur_mode != DISPLAY_MODE)
    {
        func_hold = 0;
    }
}

void picker_fsm_double_click_handler()
{
    picker_fsm_next_state();
}


void picker_fsm_set_hsv(hsv *src)
{
    if(src != NULL)
    {
        hsv_copy(src, &(fsm_inst.hsv));
        hsv_to_rgb(src, &damn);
        
        fsm_inst.updated = 1;
        picker_state_exec(fsm_inst.cur_mode);
    }
    else
    {
        NRF_LOG_INFO("No entry was found");
    }
}