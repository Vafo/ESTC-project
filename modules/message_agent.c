#include "message_agent.h"

ret_code_t message_agent_init(message_agent_ctx *msg_agent_ctx, \
                            const char *agent_name, \
                            message_agent_msg_handler msg_handler, \
                            message_agent_msg_handled_callback msg_callback, \
                            message_agent_msg_handled_callback_final msg_callback_final)
{
    ASSERT(msg_agent_ctx != NULL);
    ASSERT(msg_handler != NULL);

    msg_agent_ctx->agent.agent_name = agent_name;
    msg_agent_ctx->agent.msg_handler = msg_handler;
    msg_agent_ctx->agent.msg_callback = msg_callback;
    msg_agent_ctx->agent.msg_callback_final = msg_callback_final;
    msg_agent_ctx->msg_core = NULL;

    return NRF_SUCCESS;
}


ret_code_t message_agent_bind_to_core(message_agent_ctx *msg_agent_ctx, message_core_t *msg_core)
{
    ASSERT(msg_agent_ctx != NULL);
    ASSERT(msg_core != NULL);
    // Maybe add functionality that removes bindings to previous msg_core
    msg_agent_ctx->msg_core = msg_core;

    return NRF_SUCCESS;
}


ret_code_t message_agent_listen_to(message_agent_ctx *msg_agent_ctx, uint8_t event_id)
{
    ASSERT(msg_agent_ctx != NULL);
    
    ret_code_t ret_code;

    ret_code = message_core_add_agent_listener(msg_agent_ctx->msg_core, &(msg_agent_ctx->agent), event_id);
    return ret_code;
}


ret_code_t message_agent_send_msg(message_agent_ctx *msg_agent_ctx, uint8_t event_id, void *data, uint32_t *num_of_listeners)
{
    ASSERT(msg_agent_ctx != NULL);
    message_obj_t message = {
        .data = data,
        .event_id = event_id,
        .sender = &(msg_agent_ctx->agent)
    };
    ret_code_t ret_code;

    ret_code = message_core_enqueue_message(msg_agent_ctx->msg_core, &message);

    if(ret_code == NRF_SUCCESS && num_of_listeners != NULL)
    {
        message_agent_t *(*message_handling_agents)[MESSAGE_AGENT_PER_EVENT_NUM] = msg_agent_ctx->msg_core->message_handling_agents;
        message_agent_t **listener;
        uint8_t agent_handler_idx;

        *num_of_listeners = 0;
        listener = message_handling_agents[event_id];
        for(agent_handler_idx = 0; agent_handler_idx < MESSAGE_AGENT_PER_EVENT_NUM; agent_handler_idx++)
        {
            if(listener[agent_handler_idx] != NULL)
            {
                (*num_of_listeners)++;
            }
        }
    } 

    return ret_code;
}