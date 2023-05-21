#include "blink_hal.h"
#include "main_logs.h"

#include "app_error.h"
#include "app_error_weak.h"

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    while(1)
    {
        NRF_LOG_INFO("%s:%d", p_file_name, line_num);
        led_on(LED_1_IDX);
        NRF_LOG_PROCESS();
        LOG_BACKEND_USB_PROCESS();
    }
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    while(1)
    {
        NRF_LOG_INFO("Received a fault! id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);
        led_on(LED_1_IDX);
        NRF_LOG_PROCESS();
        LOG_BACKEND_USB_PROCESS();
    }
}


void app_error_handler_bare(uint32_t error_code)
{
    while(1)
    {
        NRF_LOG_INFO("Received an error: 0x%08x!", error_code);
        led_on(LED_1_IDX);
        NRF_LOG_PROCESS();
        LOG_BACKEND_USB_PROCESS();
    }
}