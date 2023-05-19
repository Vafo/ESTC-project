#ifndef USBD_CLI_H
#define USBD_CLI_H

#include "nordic_common.h"
#include "nrf_section.h"


#define USBD_CLI_NO_ARG(arg) ((arg).begin == (arg).end) ? 1 : 0 
#define USBD_CLI_RETURN_ON_NO_ARG(arg)  \
do {                                    \
if(USBD_CLI_NO_ARG(arg))                \
{                                       \
    return NRF_ERROR_INVALID_PARAM;     \
} } while(0)

#define USBD_CLI_LINE_BREAK "\r\n"
#define USBD_CLI_LINE_BEGIN "> "

typedef struct usbd_cli_cmd_cb_s usbd_cli_cmd_cb_t;

typedef ret_code_t (*usbd_cli_cmd_handler_t)(char *args, usbd_cli_cmd_cb_t *p_usbd_cli_cb);

typedef struct
{
    char *begin;
    char *end;
} str_interval_t;

struct usbd_cli_cmd_cb_s
{
    uint8_t is_delayed;
};

typedef struct
{
    char *p_cmd_name;
    char *p_synopsis;
    char *p_help_desc;
    usbd_cli_cmd_handler_t cmd_handler;
} usbd_cli_cmd_entry_t;

#if ESTC_USB_CLI_ENABLED == 1
void usbd_cli_init();
bool usbd_cli_process();
void usbd_cli_write(char *p_buf, size_t length);

str_interval_t usbd_cli_parse_next_arg(char *arg_str);
int usbd_cli_arg_to_int(str_interval_t arg);

#define USBD_CLI_ADD_COMMAND(cmd_name, synopsis, help_desc, handler)              \
    static usbd_cli_cmd_entry_t CONCAT_2(m_usbd_cli_user_cmd_, cmd_name) = {  \
        .p_cmd_name = STRINGIFY(cmd_name),                              \
        .p_synopsis = synopsis,                                         \
        .p_help_desc = help_desc,                                       \
        .cmd_handler = handler                                          \
    };                                                                  \
    NRF_SECTION_ITEM_REGISTER(usbd_cli_cmds,                            \
        usbd_cli_cmd_entry_t *CONCAT_2(p_usbd_cli_user_cmd_, cmd_name)) \
        = &CONCAT_2(m_usbd_cli_user_cmd_, cmd_name)
#else

#define usbd_cli_init()
#define usbd_cli_process()
#define usbd_cli_write(p_buf, length)

#define USBD_CLI_ADD_COMMAND(cmd_name, help_desc, handler) 
#endif // ESTC_USB_CLI_ENABLED == 1

#endif