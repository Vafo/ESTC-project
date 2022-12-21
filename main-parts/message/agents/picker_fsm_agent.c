#include "main_picker_fsm.h"
#include "picker_fsm_agent.h"


static message_agent_ctx picker_fsm_agent_ctx;
static char picker_fsm_name[] = "Picker FSM module";

static ret_code_t picker_fsm_agent_msg_handler(message_obj_t *message)
{
    uint8_t event_id = message->event_id;

    switch (event_id)
    {
    case PRESS_EVENT:
        picker_fsm_press_handler();
        break;
    
    case RELEASE_EVENT:
        picker_fsm_release_handler();
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
    
    return NRF_SUCCESS;
}

hsv hsv_tmp;
uint8_t hsv_tmp_mutex;

void picker_fsm_save_hsv(hsv *src)
{
    hsv_copy(src, &hsv_tmp);
    message_agent_send_msg(&picker_fsm_agent_ctx, SAVE_LED_EVENT, &hsv_tmp, NULL);
}

void picker_fsm_get_hsv()
{
    message_agent_send_msg(&picker_fsm_agent_ctx, GET_LED_EVENT, NULL, NULL);
}