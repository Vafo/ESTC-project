#ifndef MESSAGE_CORE_H
#define MESSAGE_CORE_H

#include "nrf_queue.h"

#define MESSAGE_QUEUE_SIZE 32
#define MESSAGE_AGENT_PER_EVENT_NUM 4


typedef struct {
    void *data;
    void *sender;       // message_agent_t * - Can not do it properly, because of circular dependency declaration
    uint8_t event_id;
} message_obj_t;

typedef uint8_t (*message_agent_msg_handler)(message_obj_t *message);                   // Agent message handler
typedef uint8_t (*message_agent_msg_handled_callback)(message_obj_t *message);          // Called per agent processed
typedef uint8_t (*message_agent_msg_handled_callback_final)(message_obj_t *message);    // Called on message handled by all agents

typedef struct {
    const char *agent_name;
    message_agent_msg_handler msg_handler;
    message_agent_msg_handled_callback msg_callback;               
    message_agent_msg_handled_callback_final msg_callback_final;   
} message_agent_t;


typedef struct {
    const nrf_queue_t *_message_queue;
    message_agent_t *(*message_handling_agents)[MESSAGE_AGENT_PER_EVENT_NUM];
    uint8_t events_num;

} message_core_t;


#define MESSAGE_CORE_DEF(_name, _events_num)                                \
    NRF_QUEUE_DEF(message_obj_t,  CONCAT_2(_name, _queue_inst),             \
                    MESSAGE_QUEUE_SIZE, NRF_QUEUE_MODE_OVERFLOW);           \
    message_agent_t *CONCAT_2(_name, _agents_event_handlers)                \
                    [_events_num][MESSAGE_AGENT_PER_EVENT_NUM];             \
    message_core_t _name = {                                                \
        ._message_queue = &CONCAT_2(_name, _queue_inst),                    \
        .message_handling_agents = CONCAT_2(_name, _agents_event_handlers), \
        .events_num = _events_num ,                                         \
    }                                                      

ret_code_t message_core_process_message(message_core_t *msg_core);
ret_code_t message_core_enqueue_message(message_core_t *msg_core, message_obj_t *message);

ret_code_t message_core_add_agent_listener(message_core_t *msg_core, message_agent_t *agent, uint8_t event_id);

// Register message_agents (handlers) - Done by agents
// Enqueue
// Dequeue

// NRF_QUEUE_DEF()

#endif