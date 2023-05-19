#include "stdlib.h"

#include "cli_agent.h"
#include "usbd_cli/usbd_cli.h"

#include "hsv_rgb.h"

static message_agent_ctx cli_agent_ctx;
static char cli_name[] = "USB CLI module";

static rgb rgb_data;
static hsv hsv_data;

ret_code_t cli_agent_msg_handler(message_obj_t *message)
{
    uint8_t event_id = message->event_id;

    switch(event_id)
    {

    }

    return NRF_SUCCESS;
}

ret_code_t cli_agent_init(message_core_t *msg_core)
{
    message_agent_init(&cli_agent_ctx, cli_name, cli_agent_msg_handler, NULL, NULL);
    message_agent_bind_to_core(&cli_agent_ctx, msg_core);

    // message_agent_listen_to(&picker_fsm_agent_ctx, LOADED_LED_EVENT);
    
    return NRF_SUCCESS;
}

ret_code_t cmd_rgb(char *args, usbd_cli_cmd_cb_t *p_usbd_cli_cb)
{
    str_interval_t arg = {
        .end = args
    };
    
    arg = usbd_cli_parse_next_arg(arg.end);
    USBD_CLI_RETURN_ON_NO_ARG(arg);
    rgb_data.r = MIN(((float) usbd_cli_arg_to_int(arg)) / 255, 1);
    
    arg = usbd_cli_parse_next_arg(arg.end);
    USBD_CLI_RETURN_ON_NO_ARG(arg);
    rgb_data.g = MIN(((float) usbd_cli_arg_to_int(arg)) / 255, 1);
    
    arg = usbd_cli_parse_next_arg(arg.end);
    USBD_CLI_RETURN_ON_NO_ARG(arg);
    rgb_data.b = MIN(((float) usbd_cli_arg_to_int(arg)) / 255, 1);

    rgb_to_hsv(&rgb_data, &hsv_data);
    message_agent_send_msg(&cli_agent_ctx, LOADED_LED_EVENT, &hsv_data, NULL);
    
    usbd_cli_write("Updated RGB value" USBD_CLI_LINE_BREAK, sizeof("Updated RGB value" USBD_CLI_LINE_BREAK));
    
    return NRF_SUCCESS;
}

USBD_CLI_ADD_COMMAND(RGB, "RGB <red> <green> <blue>", "Sets current color of LED. Max value - 255", cmd_rgb);
// USBD_CLI_ADD_COMMAND(HSV);