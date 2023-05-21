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

#ifndef LED_SERVICE_H__
#define LED_SERVICE_H__

#include <stdint.h>

#include "ble.h"
#include "sdk_errors.h"

// TODO: 1. Generate random BLE UUID (Version 4 UUID) and define it in the following format:
// A5DBxxxx-03AB-450D-B840-4B3F25293BAD
#define LED_BASE_UUID { 0xAD, 0x3B, 0x29, 0x25, 0x3F, 0x4B,    \
                         /* - */ 0x40, 0xB8,                    \
                         /* - */ 0x0D, 0x45,                    \
                         /* - */ 0xAB, 0x03,                    \
                         /* - */ 0x00, 0x00, 0xDB, 0xA5 }       


// TODO: 2. Pick a random service 16-bit UUID and define it:
#define LED_SERVICE_UUID 0xABBA

// TODO: 3. Pick a characteristic UUID and define it:
#define LED_GATT_CHAR_LED_SET 0xABBB
#define LED_GATT_CHAR_LED_STATE 0xABBC

#define LED_SERVICE_HVN_QUEUE_SIZE 2

#define LED_SERVICE_DEF(_name)                                                                  \
static ble_led_service_t _name;                                                                     \
NRF_SDH_BLE_OBSERVER(_name ## _ble_obs,                                                             \
                     BLE_ADV_BLE_OBSERVER_PRIO,                                                     \
                     led_ble_service_on_ble_event, &_name)

#define LED_SERVICE_DECL(_name)                                                                  \
extern ble_led_service_t _name;       

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} ble_led_set_char_value_t;

typedef struct
{
    uint16_t service_handle;
    uint16_t connection_handle;
    uint16_t hvn_available_queue_element_count;
    uint8_t inidication_free;
    // TODO: 6.3. Add handles for characterstic (type: ble_gatts_char_handles_t)
    ble_gatts_char_handles_t char_led_set;
    ble_gatts_char_handles_t char_led_state;
} ble_led_service_t;


ret_code_t led_ble_service_init(ble_led_service_t *service);

void led_ble_service_on_ble_event(const ble_evt_t *ble_evt, void *ctx);

ret_code_t led_ble_service_led_state_update(ble_led_service_t *service);

ret_code_t led_ble_service_led_state_notify(ble_led_service_t *service);

ret_code_t led_ble_service_led_set_save_value(ble_led_service_t *service);


#endif /* LED_SERVICE_H__ */