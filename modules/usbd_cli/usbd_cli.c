#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>

#include "nordic_common.h"

#include "nrf_log.h"
#include "nrf_queue.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"

#include "usbd_cli.h"

/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

#define READ_SIZE NRF_DRV_USBD_EPSIZE
#define CMD_BUFFER_SIZE 256

#if ESTC_USB_CLI_ENABLED == 1

static char m_rx_buffer[READ_SIZE];
static char m_tx_buffer[NRF_DRV_USBD_EPSIZE];
static char m_cmd_buffer[CMD_BUFFER_SIZE];

static size_t cmd_buffer_idx;

static uint8_t m_usb_tx_available;
static uint8_t m_usb_port_open;
static struct
{
    uint8_t is_ready;
    char *arg_start;
} m_usbd_cmd_buf_cb;


static usbd_cli_cmd_cb_t m_usbd_cmd_cb = {
    .is_delayed = 0
};

NRF_SECTION_DEF(usbd_cli_cmds, usbd_cli_cmd_entry_t *);
#define USBD_CLI_CMD_SECTION_ITEM_GET(i) NRF_SECTION_ITEM_GET(usbd_cli_cmds, usbd_cli_cmd_entry_t *, (i))
#define USBD_CLI_CMD_SECTION_ITEM_COUNT  NRF_SECTION_ITEM_COUNT(usbd_cli_cmds, usbd_cli_cmd_entry_t *)

NRF_QUEUE_DEF(char, m_echo_buffer_queue, 2 * NRF_DRV_USBD_EPSIZE, NRF_QUEUE_MODE_OVERFLOW);


static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event);

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);


static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
    {
        ret_code_t ret;

        NRF_LOG_INFO("Port open");
        nrf_queue_reset(&m_echo_buffer_queue);
        
        ret = app_usbd_cdc_acm_read_any(&usb_cdc_acm, m_rx_buffer, READ_SIZE);

        m_usb_tx_available = 1;
        m_usb_port_open = 1;
        
        m_usbd_cmd_cb.is_delayed = 0;

        m_usbd_cmd_buf_cb.is_ready = 0;
        m_usbd_cmd_buf_cb.arg_start = NULL;
        cmd_buffer_idx = 0;
        m_cmd_buffer[0] = '\0';

        nrf_queue_write(&m_echo_buffer_queue, USBD_CLI_LINE_BEGIN, sizeof(USBD_CLI_LINE_BEGIN));
        
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
    {
        m_usb_port_open = 0;
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
    {
        NRF_LOG_INFO("tx done");
        m_usb_tx_available = 1;
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
    {
        ret_code_t ret;
        do
        {
            size_t copy_len;
            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(&usb_cdc_acm);
            NRF_LOG_INFO("rx size: %d", size);

            /* It's the simple version of an echo. Note that writing doesn't
             * block execution, and if we have a lot of characters to read and
             * write, some characters can be missed.
             */
            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n')
            {
                nrf_queue_write(&m_echo_buffer_queue, USBD_CLI_LINE_BREAK, \
                                        sizeof(USBD_CLI_LINE_BREAK));

                m_cmd_buffer[cmd_buffer_idx] = '\0';
                m_usbd_cmd_buf_cb.is_ready = 1;

                char *p_buf = m_cmd_buffer;
                while(*p_buf != ' ' && *p_buf != '\0')
                    p_buf++;

                m_usbd_cmd_buf_cb.arg_start = p_buf;

                cmd_buffer_idx = 0;
            }
            else
            {
                nrf_queue_write(&m_echo_buffer_queue, m_rx_buffer, size);
                copy_len = size;
                if(cmd_buffer_idx + size >= CMD_BUFFER_SIZE)
                {
                    copy_len = CMD_BUFFER_SIZE - cmd_buffer_idx;
                }
                memcpy(&m_cmd_buffer[cmd_buffer_idx], m_rx_buffer, copy_len);

                if(cmd_buffer_idx + size >= CMD_BUFFER_SIZE)
                {
                    cmd_buffer_idx = 0;
                    copy_len = size - copy_len;
                    memcpy(&m_cmd_buffer[cmd_buffer_idx], m_rx_buffer + (size - copy_len), copy_len);
                }

                cmd_buffer_idx += copy_len;
            }

            /* Fetch data until internal buffer is empty */
            ret = app_usbd_cdc_acm_read_any(&usb_cdc_acm,
                                            m_rx_buffer,
                                            READ_SIZE);
        } while (ret == NRF_SUCCESS);
        
        break;
    }
    default:
        break;
    }
}

static void usb_cli_process_echo()
{
    if(!m_usb_port_open)
    {
        return;
    }

    size_t qsize;
    while(!nrf_queue_is_empty(&m_echo_buffer_queue))
    {
        qsize = nrf_queue_out(&m_echo_buffer_queue, m_tx_buffer, NRF_DRV_USBD_EPSIZE);

        usbd_cli_write(m_tx_buffer, qsize);
    }
    
}

static void usb_cli_process_cmd()
{
    if(m_usbd_cmd_cb.is_delayed)
    {
        return;
    }

    if(!m_usbd_cmd_buf_cb.is_ready)
    {
        return;
    }

    uint32_t idx;
    usbd_cli_cmd_entry_t *cmd_entry;

    char tmp_char = *m_usbd_cmd_buf_cb.arg_start;
    *m_usbd_cmd_buf_cb.arg_start = '\0';

    for(idx = 0; idx < USBD_CLI_CMD_SECTION_ITEM_COUNT; idx++)
    {
        cmd_entry = *USBD_CLI_CMD_SECTION_ITEM_GET(idx);
        if(strcmp(cmd_entry->p_cmd_name, m_cmd_buffer) == 0)
        {
            *m_usbd_cmd_buf_cb.arg_start = tmp_char;
            cmd_entry->cmd_handler(m_usbd_cmd_buf_cb.arg_start, &m_usbd_cmd_cb);
            break;
        }
    }

    // No command was found
    if(idx == USBD_CLI_CMD_SECTION_ITEM_COUNT)
    {
        usbd_cli_write("Unknown command" USBD_CLI_LINE_BREAK, sizeof("Unknown command" USBD_CLI_LINE_BREAK));
    }

    m_usbd_cmd_buf_cb.is_ready = 0;

    usbd_cli_write(USBD_CLI_LINE_BEGIN, sizeof(USBD_CLI_LINE_BEGIN));
}

char hello_msg[] = "HELO";

bool usbd_cli_process()
{
    while (app_usbd_event_queue_process())
    {
        /* Nothing to do */
    }
    usb_cli_process_echo();
    usb_cli_process_cmd();
    return true;
}


void usbd_cli_write(char *p_buf, size_t length)
{
    if(!m_usb_port_open)
        return;


    ret_code_t ret;
    size_t offset = 0;
    size_t tx_len;

    while (offset < length)
    {
        tx_len = length - offset;

        if(tx_len > NRFX_USBD_EPSIZE)
        {
            tx_len = NRFX_USBD_EPSIZE;
        }

        m_usb_tx_available = 0;
        ret = app_usbd_cdc_acm_write(&usb_cdc_acm, p_buf + offset, tx_len);
        APP_ERROR_CHECK(ret);

        while(!m_usb_tx_available)
        {
            app_usbd_event_queue_process();
        }
        
        offset += tx_len;
    }
}

void usbd_cli_init()
{
    NRF_LOG_INFO("usbd_cli initialization");
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    m_usb_port_open = 0;
    APP_ERROR_CHECK(ret);
}


ret_code_t cmd_help(char *args, usbd_cli_cmd_cb_t *p_usbd_cli_cb)
{
    size_t idx;
    usbd_cli_cmd_entry_t *p_cmd_entry;
    for(idx = 0; idx < USBD_CLI_CMD_SECTION_ITEM_COUNT; idx++)
    {
        p_cmd_entry = *USBD_CLI_CMD_SECTION_ITEM_GET(idx);
        
        usbd_cli_write(p_cmd_entry->p_cmd_name, strlen(p_cmd_entry->p_cmd_name));
        usbd_cli_write(": ", sizeof(": "));
        usbd_cli_write(USBD_CLI_LINE_BREAK "Synopsis: ", strlen(USBD_CLI_LINE_BREAK "Synopsis: "));
        usbd_cli_write(p_cmd_entry->p_synopsis, strlen(p_cmd_entry->p_synopsis));
        usbd_cli_write(USBD_CLI_LINE_BREAK "Description: ", strlen(USBD_CLI_LINE_BREAK "Description: "));
        usbd_cli_write(p_cmd_entry->p_help_desc, strlen(p_cmd_entry->p_help_desc));
        usbd_cli_write(USBD_CLI_LINE_BREAK USBD_CLI_LINE_BREAK, sizeof(USBD_CLI_LINE_BREAK USBD_CLI_LINE_BREAK));
    }

    return NRF_SUCCESS;
}

str_interval_t usbd_cli_parse_next_arg(char *arg_str)
{
    char *ptr = arg_str;
    while(isspace(*ptr) && *ptr != '\0')
        ptr++;
        
    arg_str = ptr;
    while(!isspace(*ptr) && *ptr != '\0')
        ptr++;

    str_interval_t res = {
        .begin = arg_str,
        .end = ptr
    };

    return res;
}

int usbd_cli_arg_to_int(str_interval_t arg)
{
    int res;
    char tmp = *arg.end;
    *arg.end = '\0';
    res = atoi(arg.begin);
    *arg.end = tmp;
    return res;
}

USBD_CLI_ADD_COMMAND(help, "help", "Display information about available commands", cmd_help);

#endif // ESTC_USB_CLI_ENABLED == 1