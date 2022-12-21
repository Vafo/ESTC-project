#include "message_core.h"

ret_code_t message_core_add_agent_listener(message_core_t *msg_core, message_agent_t *agent, uint8_t event_id)
{
    ASSERT(msg_core != NULL);
    ASSERT(agent != NULL);
    ASSERT(0 <= event_id && event_id < msg_core->events_num);

    message_agent_t *(*message_handling_agents)[MESSAGE_AGENT_PER_EVENT_NUM] = msg_core->message_handling_agents;
    message_agent_t **listener;
    uint8_t agent_handler_idx;
    ret_code_t ret_code = NRF_ERROR_NO_MEM;

    listener = message_handling_agents[event_id];
    for(agent_handler_idx = 0; agent_handler_idx < MESSAGE_AGENT_PER_EVENT_NUM; agent_handler_idx++)
    {
        if(listener[agent_handler_idx] == NULL)
        {
            listener[agent_handler_idx] = agent;
            ret_code = NRF_SUCCESS;
            break;
        }
    }

    return ret_code;
}

ret_code_t message_core_enqueue_message(message_core_t *msg_core, message_obj_t *message)
{
    const nrf_queue_t *msg_queue = msg_core->_message_queue;
    ASSERT((msg_queue) != NULL);
    ASSERT((msg_queue)->element_size == sizeof(message_obj_t));
    uint8_t event_id = message->event_id;
    ASSERT(0 <= event_id && event_id < msg_core->events_num);

    ret_code_t ret_code;
    ret_code = nrf_queue_push(msg_queue, (void *) message);

    return ret_code;
}

ret_code_t message_core_process_message(message_core_t *msg_core)
{
    ASSERT(msg_core != NULL);

    const nrf_queue_t *msg_queue = msg_core->_message_queue;
    message_agent_t *(*message_handling_agents)[MESSAGE_AGENT_PER_EVENT_NUM] = msg_core->message_handling_agents;

    ASSERT((msg_queue) != NULL);
    ASSERT((msg_queue)->element_size == sizeof(message_obj_t));

    ret_code_t ret_code;
    message_obj_t message;
    uint8_t event_id;
    uint8_t agent_handler_idx;
    message_agent_t *sender;
    message_agent_t **listener;

    ret_code = nrf_queue_pop(msg_queue, (void *) &message);

    if(ret_code == NRF_SUCCESS)
    {
        event_id = message.event_id;
        sender = message.sender;
        
        ASSERT(sender != NULL);
        ASSERT(0 <= event_id && event_id < msg_core->events_num);
        listener = message_handling_agents[event_id];

        for(agent_handler_idx = 0; agent_handler_idx < MESSAGE_AGENT_PER_EVENT_NUM; agent_handler_idx++)
        {
            if(listener[agent_handler_idx] != NULL)
            {
                ASSERT(listener[agent_handler_idx]->msg_handler != NULL);
                listener[agent_handler_idx]->msg_handler(&message);

                if(sender->msg_callback != NULL)
                {
                    sender->msg_callback(&message);
                }
            }
        }

        if(sender->msg_callback_final != NULL)
        {
            sender->msg_callback_final(&message);
        }
    }

    return ret_code;
}