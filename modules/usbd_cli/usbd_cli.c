#include <stdbool.h>
#include <stdint.h>

#include "nordic_common.h"

#include "nrf_log.h"
#include "nrf_queue.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"

#include "usbd_cli.h"

#define READ_SIZE NRF_DRV_USBD_EPSIZE

static char m_rx_buffer[READ_SIZE];
static char m_tx_buffer[NRF_DRV_USBD_EPSIZE];
static uint8_t m_usb_tx_available;

NRF_QUEUE_DEF(char, m_echo_buffer_queue, 2 * NRF_DRV_USBD_EPSIZE, NRF_QUEUE_MODE_OVERFLOW);

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event);

/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);


static void usb_cli_process_echo()
{
    size_t q_size;
    if(!m_usb_tx_available || nrf_queue_is_empty(&m_echo_buffer_queue))
    {
        return;
    }

    q_size = nrf_queue_out(&m_echo_buffer_queue, m_tx_buffer, NRF_DRV_USBD_EPSIZE);
    m_usb_tx_available = 0;
    app_usbd_cdc_acm_write(&usb_cdc_acm, m_tx_buffer, q_size);
}

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
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
    {
        m_usb_tx_available = 0;
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
            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(&usb_cdc_acm);
            NRF_LOG_INFO("rx size: %d", size);

            /* It's the simple version of an echo. Note that writing doesn't
             * block execution, and if we have a lot of characters to read and
             * write, some characters can be missed.
             */
            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n')
            {
                nrf_queue_write(&m_echo_buffer_queue, "\r\n", 2);
            }
            else
            {
                nrf_queue_write(&m_echo_buffer_queue, m_rx_buffer, READ_SIZE);
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

bool usbd_cli_process()
{
    while (app_usbd_event_queue_process())
    {
        /* Nothing to do */
    }
    usb_cli_process_echo();
    // usb_cli_process_cmd();
    return true;
}


void usbd_cli_init()
{
    NRF_LOG_INFO("usbd_cli initialization");
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);
}