#include "stdlib.h"

#include "nrfx.h"
#include "nrf_log.h"

#include "ble_agent.h"
#include "ble/led_service.h"

#include "hsv_rgb.h"

static message_agent_ctx ble_agent_ctx;
static char ble_name[] = "BLE module";

static rgb rgb_data;
static hsv hsv_data;

ret_code_t ble_agent_msg_handler(message_obj_t *message)
{
    uint8_t event_id = message->event_id;

    switch(event_id)
    {

    }

    return NRF_SUCCESS;
}

ret_code_t led_ble_service_led_set_save_value(ble_led_service_t *service)
{
    ret_code_t ret;
    uint16_t conn_handle = service->connection_handle;
    uint16_t char_led_set_val_handle = service->char_led_set.value_handle;
    ble_led_set_char_value_t val;
    ble_gatts_value_t gatts_rgb_value = {
        .p_value = (uint8_t *) &val,
        .len = sizeof(ble_led_set_char_value_t)
    };

    ret = sd_ble_gatts_value_get(conn_handle, char_led_set_val_handle, &gatts_rgb_value);
    if(ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("%s: invalid retcode %d", __func__, ret);
        return ret;
    }

    // Message is of size 3 byte (255, 255, 255)
    // Convert it to floats
    NRF_LOG_INFO("Checking length");
    if(gatts_rgb_value.len != sizeof(ble_led_set_char_value_t))
    {
        NRF_LOG_INFO("Invalid length");
        return NRFX_ERROR_INVALID_LENGTH;
    }
    NRF_LOG_INFO("Copying data");
    
    rgb_data.r = ((float) val.red)  / 255;
    rgb_data.g = ((float) val.green)  / 255;
    rgb_data.b = ((float) val.blue)  / 255;

    rgb_to_hsv(&rgb_data, &hsv_data);

    ret = message_agent_send_msg(&ble_agent_ctx, SAVE_LED_EVENT, &hsv_data, NULL);
    NRF_LOG_INFO("SAVE_LED_EVENT SENT %d", ret);
    ret = message_agent_send_msg(&ble_agent_ctx, GET_LED_EVENT, NULL, NULL);
    NRF_LOG_INFO("GET_LED_EVENT SENT %d", ret);

    return NRFX_SUCCESS;
}

ret_code_t ble_agent_init(message_core_t *msg_core)
{
    message_agent_init(&ble_agent_ctx, ble_name, ble_agent_msg_handler, NULL, NULL);
    message_agent_bind_to_core(&ble_agent_ctx, msg_core);

    // message_agent_listen_to(&picker_fsm_agent_ctx, LOADED_LED_EVENT);
    
    return NRF_SUCCESS;
}