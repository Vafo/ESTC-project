#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "blink_hal.h"

#include "main_click.h"
#include "main_logs.h"
#include "main_pwm.h"
#include "main_picker_fsm.h"
#include "main_message_core.h"

#include "usbd_cli/usbd_cli.h"

int main(void)
{
    NRF_LOG_INFO("Starting application");
    
    main_message_core_init();
    led_init_all();
    picker_fsm_init();
    NRF_LOG_INFO("Starting up the test project with USB logging");
    main_logs_init();
    main_click_init();

    usbd_cli_init();

    while (true)
    {   
        main_message_core_process_message();
        usbd_cli_process();

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
    

}


/**
 *@}
 **/
