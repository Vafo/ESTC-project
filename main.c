#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "nrf_soc.h"
#include "nrf_pwr_mgmt.h"

#include "blink_hal.h"

#include "main_click.h"
#include "main_logs.h"
#include "main_pwm.h"
#include "main_picker_fsm.h"
#include "main_message_core.h"

#include "usbd_cli/usbd_cli.h"
#include "ble/main_ble.h"


/**@brief Function for initializing power management.
 */
// static void power_management_init(void)
// {
//     ret_code_t err_code;
//     err_code = nrf_pwr_mgmt_init();
//     APP_ERROR_CHECK(err_code);
// }

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        // nrf_pwr_mgmt_run();
    }
	LOG_BACKEND_USB_PROCESS();
}

void request_hfclk()
{
    uint32_t hfclk_runs = 0;
    sd_clock_hfclk_request();
    while(!hfclk_runs)
    {
        sd_clock_hfclk_is_running(&hfclk_runs);
    }
}


int main(void)
{
    NRF_LOG_INFO("Starting application");
    
    // power_management_init();
    main_message_core_init();
    led_init_all();
    main_logs_init();
    picker_fsm_init();
    main_click_init();
    main_ble_init();
    request_hfclk();
    NRF_LOG_INFO("Starting up the test project with USB logging");

    usbd_cli_init();

    while (true)
    {   
        main_message_core_process_message();
        usbd_cli_process();

        idle_state_handle();
    }
    
    return 0;

}


/**
 *@}
 **/
