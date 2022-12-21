#ifndef MESSAGE_AGENT_H
#define MESSAGE_AGENT_H

#include "message_core.h"

typedef struct {
    message_agent_t *agent;
    message_core_t *msg_core;
} message_agent_ctx;

uint8_t message_agent_init(message_agent_ctx *msg_agent_ctx, \
                            const char *agent_name, \
                            message_agent_msg_handler *msg_handler, \
                            message_agent_msg_handled_callback *msg_callback, \
                            message_agent_msg_handled_callback_final *msg_callback_final \
                            );
uint8_t message_agent_send_msg(message_agent_ctx *msg_agent_ctx, uint8_t event_id, void *data, uint32_t *num_of_listeners);
uint8_t message_agent_bind_to_core(message_agent_ctx *msg_agent_ctx, message_core_t *msg_core);


// Interface for modules to use given ptr to message_core

// Send message (provide ptr, so to store num of listeners)
// Handle (Receive) message

#endif