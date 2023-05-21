#include "main_picker_fsm.h"
#include "picker_fsm_agent.h"


static message_agent_ctx picker_fsm_agent_ctx;
static char picker_fsm_name[] = "Picker FSM module";

hsv hsv_tmp;
rgb rgb_tmp;

static ret_code_t picker_fsm_agent_msg_handler(message_obj_t *message)
{
    uint8_t event_id = message->event_id;
    const nrf_pwm_values_wave_form_t *p_wave;
    uint16_t top_val;

    switch (event_id)
    {
    case PRESS_EVENT:
        picker_fsm_press_handler();
        break;
    
    case RELEASE_EVENT:
        picker_fsm_release_handler();
        p_wave = fsm_inst.rgb_ctx->seq->values.p_wave_form;
        top_val = p_wave->counter_top;

        rgb_tmp.r = ((float) p_wave->channel_0) / top_val;
        rgb_tmp.g = ((float) p_wave->channel_1) / top_val;
        rgb_tmp.b = ((float) p_wave->channel_2) / top_val;



        rgb_to_hsv(&rgb_tmp, &hsv_tmp);
        message_agent_send_msg(&picker_fsm_agent_ctx, PICKER_LED_EVENT, &hsv_tmp, NULL);

        break;
    
    case DOUBLE_CLICK_EVENT:
        picker_fsm_double_click_handler();
        break;

    case LOADED_LED_EVENT:
        ASSERT(message->data != NULL);
        picker_fsm_set_hsv(message->data);
        break;

    default:
        // I dont know what to do in this case
        break;
    }

    return NRF_SUCCESS;
}


ret_code_t picker_fsm_agent_init(message_core_t *msg_core)
{
    message_agent_init(&picker_fsm_agent_ctx, picker_fsm_name, picker_fsm_agent_msg_handler, NULL, NULL);
    message_agent_bind_to_core(&picker_fsm_agent_ctx, msg_core);
    
    message_agent_listen_to(&picker_fsm_agent_ctx, PRESS_EVENT);
    message_agent_listen_to(&picker_fsm_agent_ctx, RELEASE_EVENT);
    message_agent_listen_to(&picker_fsm_agent_ctx, DOUBLE_CLICK_EVENT);
    message_agent_listen_to(&picker_fsm_agent_ctx, LOADED_LED_EVENT);
    
    return NRF_SUCCESS;
}

void picker_fsm_save_hsv(hsv *src)
{
    hsv_copy(src, &hsv_tmp);
    message_agent_send_msg(&picker_fsm_agent_ctx, SAVE_LED_EVENT, &hsv_tmp, NULL);
}

void picker_fsm_get_hsv()
{
    message_agent_send_msg(&picker_fsm_agent_ctx, GET_LED_EVENT, NULL, NULL);
}