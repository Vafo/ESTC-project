#include "main_message_core.h"

#include "agents/click_agent.h"
#include "agents/picker_fsm_agent.h"
#include "agents/nvmc_agent.h"
#include "agents/cli_agent.h"
#include "agents/ble_agent.h"

MESSAGE_CORE_DEF(msg_core, MESSAGE_EVENT_NUMBER);

ret_code_t main_message_core_init()
{
    click_agent_init(&msg_core);
    picker_fsm_agent_init(&msg_core);
    nvmc_agent_init(&msg_core);
    cli_agent_init(&msg_core);
    ble_agent_init(&msg_core);

    return NRF_SUCCESS;
}


ret_code_t main_message_core_process_message()
{
    message_core_process_message(&msg_core);
    
    return NRF_SUCCESS;
}