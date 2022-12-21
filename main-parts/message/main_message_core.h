#ifndef MAIN_MESSAGE_CORE_H
#define MAIN_MESSAGE_CORE_H

#include "message_core.h"

typedef enum {
    PRESS_EVENT,
    RELEASE_EVENT,
    DOUBLE_CLICK_EVENT,
    SAVE_DATA_EVENT,

    MESSAGE_EVENT_NUMBER
} message_event;

extern message_core_t msg_core;


#endif