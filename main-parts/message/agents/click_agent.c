#include "main_click.h"
#include "click_agent.h"

static message_agent_ctx click_agent_ctx;
static char click_name[] = "Click module";


ret_code_t click_agent_msg_handler(message_obj_t *message)
{
    // Never to happen. Maybe
    return NRF_ERROR_INVALID_STATE; 
}

ret_code_t click_agent_init(message_core_t *msg_core)
{
    message_agent_init(&click_agent_ctx, click_name, click_agent_msg_handler, NULL, NULL);
    message_agent_bind_to_core(&click_agent_ctx, msg_core);
    
    return NRF_SUCCESS;
}


void on_press()
{
    message_agent_send_msg(&click_agent_ctx, PRESS_EVENT, NULL, NULL);
}

void on_release()
{
    message_agent_send_msg(&click_agent_ctx, RELEASE_EVENT, NULL, NULL);
}

void on_double_click()
{
    message_agent_send_msg(&click_agent_ctx, DOUBLE_CLICK_EVENT, NULL, NULL);
}
