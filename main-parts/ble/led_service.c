/**
 * Copyright 2022 Evgeniy Morozov
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
*/

#include "led_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"
#include "nrf_ble_qwr.h"

#include "hsv_rgb.h"

LED_SERVICE_DECL(m_led_service);

ble_uuid128_t base_uuid = {
    .uuid128 = LED_BASE_UUID
};

uint8_t m_char_led_set_desc[] = "Set RGB Value (0x??&&@@ format)";
uint8_t m_char_led_state_desc[] = "Current RGB Value (0x??&&@@ format)";

static ret_code_t led_ble_add_characteristics(ble_led_service_t *service);
static ret_code_t led_ble_check_user_need_for_hvx(uint16_t conn_handle, uint16_t cccd_handle, uint16_t hvx_type);

ret_code_t led_ble_service_init(ble_led_service_t *service)
{
    VERIFY_PARAM_NOT_NULL(service);
    ret_code_t error_code = NRF_SUCCESS;
    ble_uuid_t service_uuid = {
        .uuid = LED_SERVICE_UUID
    };

    // TODO: 4. Add service UUIDs to the BLE stack table using `sd_ble_uuid_vs_add`
    error_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(error_code);

    // TODO: 5. Add service to the BLE stack using `sd_ble_gatts_service_add`
    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &service->service_handle);
    APP_ERROR_CHECK(error_code);

    NRF_LOG_DEBUG("%s:%d | Service UUID: 0x%04x", __FUNCTION__, __LINE__, service_uuid.uuid);
    NRF_LOG_DEBUG("%s:%d | Service UUID type: 0x%02x", __FUNCTION__, __LINE__, service_uuid.type);
    NRF_LOG_DEBUG("%s:%d | Service handle: 0x%04x", __FUNCTION__, __LINE__, service->service_handle);

    service->connection_handle = BLE_CONN_HANDLE_INVALID;

    return led_ble_add_characteristics(service);
}

void led_ble_service_on_ble_event(const ble_evt_t *p_ble_evt, void *ctx)
{
    // ret_code_t err_code = NRF_SUCCESS;
    ble_led_service_t *p_led_service = (ble_led_service_t *) ctx;
    // ble_gatts_evt_write_t led_gatts_write_evt;
    // ble_gatts_value_t led_gatts_value;
    // ble_led_rgb_value_t *p_rgb_data;
    uint16_t m_conn_handle;

    ble_gatts_evt_rw_authorize_request_t rw_auth_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            p_led_service->connection_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            p_led_service->connection_handle = m_conn_handle;

            p_led_service->hvn_available_queue_element_count = LED_SERVICE_HVN_QUEUE_SIZE;
            p_led_service->inidication_free = 1;
            break;

        /*
        case BLE_GATTS_EVT_WRITE:
            led_gatts_write_evt = p_ble_evt->evt.gatts_evt.params.write;
            m_conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;

            if(m_conn_handle != p_led_service->connection_handle)
            {
                break;
            }
            // NRF_LOG_INFO("AUTH %d OP %d", led_gatts_write_evt.auth_required, led_gatts_write_evt.op);
            if(led_gatts_write_evt.auth_required != 1)
            {
                break;
            }
            
            if(led_gatts_write_evt.len != sizeof(ble_led_rgb_value_t) || led_gatts_write_evt.offset != 0)
            {
                NRF_LOG_INFO("Bad length in Write Command Size %d != %d offset %d", \
                    led_gatts_write_evt.len, sizeof(ble_led_rgb_value_t), \
                    led_gatts_write_evt.offset);
                break;
            }

            if(led_gatts_write_evt.uuid.uuid == LED_GATT_CHAR_LED_SET && \
               led_gatts_write_evt.handle == p_led_service->char_led_set.value_handle)
            {
                led_gatts_value.len = led_gatts_write_evt.len;
                led_gatts_value.offset = led_gatts_write_evt.offset;
                led_gatts_value.p_value = led_gatts_write_evt.data;

                p_rgb_data = (ble_led_rgb_value_t *) led_gatts_value.p_value;
                NRF_LOG_INFO("Write evt: received value (%d, %d, %d)", p_rgb_data->red, p_rgb_data->green, p_rgb_data->blue);
                
                sd_ble_gatts_value_set(m_conn_handle, p_led_service->char_led_set.value_handle, &led_gatts_value);
                led_ble_service_led_set_save_value(p_led_service);
            }

            break;
        */

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            rw_auth_evt = p_ble_evt->evt.gatts_evt.params.authorize_request;
            if(rw_auth_evt.request.write.handle == p_led_service->char_led_set.value_handle)
            {
                // NRF_LOG_INFO("THIS IS A HIT %d", rw_auth_evt.request.write.op);
            }

            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            m_conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
            if(m_conn_handle == p_led_service->connection_handle)
            {
                p_led_service->hvn_available_queue_element_count += \
                            p_ble_evt->evt.gatts_evt.params.hvn_tx_complete.count;
            }
            break;

        case BLE_GATTS_EVT_HVC:
            m_conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
            if(m_conn_handle == p_led_service->connection_handle)
            {
                // Check for corresponding char handle
                p_led_service->inidication_free = 1;
            }
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            m_conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
            
            if(m_conn_handle == p_led_service->connection_handle)
            {
                p_led_service->connection_handle = BLE_CONN_HANDLE_INVALID;
            }
            break;


        
        default:
            // No implementation needed.
            break;
    }
}

uint16_t led_qwr_evt_handler_t(struct nrf_ble_qwr_t * p_qwr,
                                nrf_ble_qwr_evt_t    * p_evt)
{
    ret_code_t err = BLE_GATT_STATUS_UNKNOWN;
    ble_led_service_t *p_led_service = &m_led_service;

    nrf_ble_qwr_evt_type_t evt_type = p_evt->evt_type;

    ble_led_rgb_value_t rgb_data;
    uint16_t rgb_len = sizeof(ble_led_rgb_value_t);
    NRF_LOG_INFO("LED QWR HANDLER WAS CALLED WITH evt_type %d", evt_type);
    switch(evt_type)
    {
        case NRF_BLE_QWR_EVT_AUTH_REQUEST:

            // if(p_evt->attr_handle != p_led_service->char_led_set.value_handle)
            // {
            //     break;
            // }

            err = nrf_ble_qwr_value_get(p_qwr, p_evt->attr_handle, (uint8_t *) &rgb_data, &rgb_len);
            APP_ERROR_CHECK(err);



            if(rgb_len != sizeof(ble_led_rgb_value_t))
            {
                NRF_LOG_INFO("Bad length in Write Command Size %d != %d", rgb_len, sizeof(ble_led_rgb_value_t));
                break;
            }

            NRF_LOG_INFO("QWR Write auth evt: received value (%d, %d, %d)", rgb_data.red, rgb_data.green, rgb_data.blue);

            err = BLE_GATT_STATUS_SUCCESS;
            break;

        case NRF_BLE_QWR_EVT_EXECUTE_WRITE:

            led_ble_service_led_set_save_value(p_led_service);

            break;
        
        default:

            break;
    }

    return err;
}

static ret_code_t led_ble_add_characteristics(ble_led_service_t *service)
{
    VERIFY_PARAM_NOT_NULL(service);
    ret_code_t error_code = NRF_SUCCESS;

    // LED Set Charachteristic
    ble_uuid_t char_uuid = {
        .uuid = LED_GATT_CHAR_LED_SET
    };

    // TODO: 6.1. Add custom characteristic UUID using `sd_ble_uuid_vs_add`, same as in step 4
    error_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(error_code);

    // TODO: 6.5. Configure Characteristic metadata (enable read and write)
    ble_gatts_char_md_t char_md = { 0 };
    // char_md.char_props.read = 1;
    // char_md.char_props.write_wo_resp = 1;
    char_md.char_props.write = 1;
    char_md.char_ext_props.reliable_wr = 1;         // Enable QWR 
    // char_md.char_props.auth_signed_wr = 1;
    
    // Add User Description Descriptor
    char_md.p_char_user_desc = m_char_led_set_desc;
    
    char_md.char_user_desc_size = ARRAY_SIZE(m_char_led_set_desc);
    char_md.char_user_desc_max_size = ARRAY_SIZE(m_char_led_set_desc);
    char_md.p_user_desc_md = NULL;  // Default md values of user descr attr 

    // Configures attribute metadata. For now we only specify that the attribute will be stored in the softdevice
    ble_gatts_attr_md_t attr_md = { 0 };
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.wr_auth = 1;


    // TODO: 6.6. Set read/write security levels to our attribute metadata using `BLE_GAP_CONN_SEC_MODE_SET_OPEN`
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // TODO: 6.2. Configure the characteristic value attribute (set the UUID and metadata)
    ble_gatts_attr_t attr_char_value = { 0 };
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.p_uuid = &char_uuid;

    // TODO: 6.7. Set characteristic length in number of bytes in attr_char_value structure
    attr_char_value.init_len = sizeof(ble_led_rgb_value_t);
    attr_char_value.max_len = sizeof(ble_led_rgb_value_t);

    // TODO: 6.4. Add new characteristic to the service using `sd_ble_gatts_characteristic_add`
    error_code = sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_char_value, &service->char_led_set);
    APP_ERROR_CHECK(error_code);


    // LED State Characteristic
    ble_uuid_t char_led_state_uuid = {
        .uuid = LED_GATT_CHAR_LED_STATE
    };

    error_code = sd_ble_uuid_vs_add(&base_uuid, &char_led_state_uuid.type);
    APP_ERROR_CHECK(error_code);

    ble_gatts_attr_md_t char_led_state_cccd_md = {
        .vloc = BLE_GATTS_VLOC_STACK
    };
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_led_state_cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_led_state_cccd_md.write_perm);

    ble_gatts_char_md_t char_led_state_md = {0};
    char_led_state_md.char_props.read = 1;
    char_led_state_md.char_props.notify = 1;
    char_led_state_md.p_cccd_md = &char_led_state_cccd_md;

    char_led_state_md.p_char_user_desc = m_char_led_state_desc;
    char_led_state_md.char_user_desc_size = ARRAY_SIZE(m_char_led_state_desc);
    char_led_state_md.char_user_desc_max_size = ARRAY_SIZE(m_char_led_state_desc);
    char_led_state_md.p_user_desc_md = NULL;
    
    ble_gatts_attr_md_t char_led_state_value_md = {0};
    char_led_state_value_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_led_state_value_md.read_perm);

    ble_gatts_attr_t char_led_state_value = {0};
    char_led_state_value.p_attr_md = &char_led_state_value_md;
    char_led_state_value.p_uuid = &char_led_state_uuid;
    char_led_state_value.init_len = sizeof(ble_led_rgb_value_t);
    char_led_state_value.max_len = sizeof(ble_led_rgb_value_t);
    
    error_code = sd_ble_gatts_characteristic_add(service->service_handle, &char_led_state_md, &char_led_state_value, &service->char_led_state);
    APP_ERROR_CHECK(error_code);

    return NRF_SUCCESS;
}

ret_code_t led_ble_service_led_state_update(ble_led_service_t *service, ble_led_rgb_value_t *p_value)
{
    ret_code_t error_code = NRF_SUCCESS;

    ble_gatts_value_t new_val = {
        .p_value = (uint8_t *) p_value,
        .len = sizeof(ble_led_rgb_value_t),
        .offset = 0
    };

    error_code = sd_ble_gatts_value_set(service->connection_handle, service->char_led_state.value_handle, &new_val);
    APP_ERROR_CHECK(error_code);

    error_code = led_ble_service_led_state_notify(service);
    
    if(error_code == NRF_SUCCESS)
    {
        NRF_LOG_INFO("Notified with val (%d, %d, %d)", p_value->red, p_value->green, p_value->blue);
    }

    return NRF_SUCCESS;
}

// Check for events
ret_code_t led_ble_service_led_state_notify(ble_led_service_t *service)
{
    NRF_LOG_INFO("Trying to notify ...");
    if(service->connection_handle == BLE_CONN_HANDLE_INVALID)
    {
        NRF_LOG_INFO("... connection handle is invalid");
        return BLE_ERROR_INVALID_CONN_HANDLE;
    }
    ret_code_t error_code = NRF_SUCCESS;

    error_code = led_ble_check_user_need_for_hvx(service->connection_handle, service->char_led_state.cccd_handle, BLE_GATT_HVX_NOTIFICATION);
    if(error_code != NRF_SUCCESS)
    {
        NRF_LOG_INFO("... attempted to notify client, who doesn't need it");
        return NRF_ERROR_FORBIDDEN;
    }

    ble_gatts_hvx_params_t hvx_params = {
        .handle = service->char_led_state.value_handle,
        .type = BLE_GATT_HVX_NOTIFICATION,
        .offset = 0,
        .p_data = NULL,
        .p_len = NULL   // Risky move ?
    };
    
    if(service->hvn_available_queue_element_count == 0)
    {
        NRF_LOG_INFO("No space left in hvn queue");
        return NRF_ERROR_NO_MEM;
    }
    NRF_LOG_INFO("There is space in hvn queue");

    error_code = sd_ble_gatts_hvx(service->connection_handle, &hvx_params);
    NRF_LOG_INFO("%s: retval of sd_ble_gatts_hvx : %x", __FUNCTION__, error_code);
    VERIFY_SUCCESS(error_code);
    service->hvn_available_queue_element_count--;

    return NRF_SUCCESS;
}

// ret_code_t led_ble_service_btn_state_set(ble_led_service_t *service, uint8_t *new_state)
// {
//     ret_code_t error_code = NRF_SUCCESS;

//     uint8_t m_new_state = *new_state;

//     ble_gatts_value_t new_val = {
//         .p_value = &m_new_state,
//         .len = sizeof(uint8_t),
//         .offset = 0
//     };

//     error_code = sd_ble_gatts_value_set(service->connection_handle, service->char_btn_state.value_handle, &new_val);
//     APP_ERROR_CHECK(error_code);

//     error_code = led_ble_service_btn_state_indicate(service);
    
//     if(error_code == NRF_SUCCESS)
//     {
//         NRF_LOG_INFO("Indicated with new state %d", m_new_state);
//     }

//     return NRF_SUCCESS;
// }

// ret_code_t led_ble_service_btn_state_indicate(ble_led_service_t *service)
// {
//     NRF_LOG_INFO("Trying to indicate ...");
//     if(service->connection_handle == BLE_CONN_HANDLE_INVALID)
//     {
//         NRF_LOG_INFO("... connection handle is invalid");
//         return BLE_ERROR_INVALID_CONN_HANDLE;
//     }
//     ret_code_t error_code = NRF_SUCCESS;

//     error_code = led_ble_check_user_need_for_hvx(service->connection_handle, service->char_btn_state.cccd_handle, 
//                                                     BLE_GATT_HVX_INDICATION);
//     if(error_code != NRF_SUCCESS)
//     {
//         NRF_LOG_INFO("... attempted to indicate client, who doesn't need it");
//         return NRF_ERROR_FORBIDDEN;
//     }

//     ble_gatts_hvx_params_t hvx_params = {
//         .handle = service->char_btn_state.value_handle,
//         .type = BLE_GATT_HVX_INDICATION,
//         .offset = 0,
//         .p_data = NULL,
//         .p_len = NULL   // Risky move ?
//     };
    
//     // Check for previous indication being processed
//     if(service->inidication_free == 0)
//     {
//         NRF_LOG_INFO("Indications are not available yet");
//         return NRF_ERROR_INVALID_STATE;
//     }
//     NRF_LOG_INFO("Indications are available");

//     error_code = sd_ble_gatts_hvx(service->connection_handle, &hvx_params);
//     NRF_LOG_INFO("%s: retval of sd_ble_gatts_hvx : %x", __FUNCTION__, error_code);
//     VERIFY_SUCCESS(error_code);
//     service->inidication_free = 0;

//     return NRF_SUCCESS;
// }

static ret_code_t led_ble_check_user_need_for_hvx(uint16_t conn_handle, uint16_t cccd_handle, uint16_t hvx_type)
{
    uint16_t m_cccd_value;
    ret_code_t error_code;
    
    ble_gatts_value_t cccd_value_param = {
        .len = sizeof(m_cccd_value),
        .p_value = (uint8_t *) &m_cccd_value
    };

    error_code = sd_ble_gatts_value_get( conn_handle, 
                                         cccd_handle, 
                                         &cccd_value_param);
    
    if(error_code != NRF_SUCCESS)
    {
        NRF_LOG_INFO("sd_ble_gatts_value_get retval %x", error_code);
    }
    
    // BLE_ERROR_GATTS_SYS_ATTR_MISSING is caused, whenever user haven't accessed CCCD
    VERIFY_SUCCESS(error_code);
    
    // Debug info
    if(cccd_value_param.len != sizeof(m_cccd_value))
    {
        NRF_LOG_INFO("... CCCD is more than %d bytes, it is %d", sizeof(m_cccd_value), cccd_value_param.len);
        return NRF_ERROR_INVALID_STATE;
    }
    
    // Maybe there is better way around to mitigate notifying users, who don't need it
    return m_cccd_value == hvx_type ? NRF_SUCCESS : NRF_ERROR_FORBIDDEN;
}
