#include "nvmc_agent.h"
#include "nvmc_api.h"
#include "hsv_rgb.h"

#include "main_logs.h"

static message_agent_ctx nvmc_agent_ctx;
static nvmc_state_t nvmc_state;
static char nvmc_name[] = "NVMC module";

void nvmc_agent_save_hsv(hsv *src)
{
    size_t num_bytes = sizeof(hsv);
    nvmc_api_write_next_n_bytes(&nvmc_state, (nvmc_api_byte_t *) src, num_bytes);
    rgb rgb_tmp;
    hsv_to_rgb(src, &rgb_tmp);
    NRF_LOG_INFO("SAVED R : %d | G : %d | B : %d |" , rgb_tmp.r * 1000, rgb_tmp.g * 1000, rgb_tmp.b * 1000);
}

hsv hsv_tmp = {
    .h = 0,
    .s = 1,
    .v = 1
};
void nvmc_agent_get_hsv()
{
    // nvmc_api_byte_t sign_byte = 0;

    // nvmc_api_address_t valid_loc;
    // while(sign_byte != NVMC_API_EMPTY_BYTE_SLOT)
    // {
    //     valid_loc = nvmc_api_get_cur_read_pos(&nvmc_state);
    //     nvmc_api_read_cur_n_bytes(&nvmc_state, &sign_byte, sizeof(sign_byte));
    //     nvmc_api_set_cur_read_pos(&nvmc_state, valid_loc + sizeof(hsv));
    // }
    // nvmc_api_set_cur_read_pos(&nvmc_state, valid_loc);
    // nvmc_api_read_cur_n_bytes(&nvmc_state,  (nvmc_api_byte_t *) &hsv_tmp, sizeof(hsv)); 

    message_agent_send_msg(&nvmc_agent_ctx, LOADED_LED_EVENT, &hsv_tmp, NULL);
}

ret_code_t nvmc_agent_msg_handler(message_obj_t *message)
{
    uint8_t event_id = message->event_id;

    switch (event_id)
    {

    case SAVE_LED_EVENT:
        ASSERT(message->data != NULL);
        nvmc_agent_save_hsv(message->data);
        break;

    case GET_LED_EVENT:
        nvmc_agent_get_hsv();
        break;

    default:
        // I dont know what to do in this case
        break;
    }

    return NRF_SUCCESS;
}

ret_code_t nvmc_agent_init(message_core_t *msg_core)
{
    nvmc_api_init(&nvmc_state);
    message_agent_init(&nvmc_agent_ctx, nvmc_name, nvmc_agent_msg_handler, NULL, NULL);
    message_agent_bind_to_core(&nvmc_agent_ctx, msg_core);

    message_agent_listen_to(&nvmc_agent_ctx, SAVE_LED_EVENT);
    message_agent_listen_to(&nvmc_agent_ctx, GET_LED_EVENT);

    return NRF_SUCCESS;
}