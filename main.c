#include <stdbool.h>
#include <stdint.h>
#include "boards.h"

#include "main_blinking.h"
#include "main_click.h"
#include "main_logs.h"
#include "main_click.h"


int main(void)
{
    led_init_all();
    pwm_led_init();
    
    NRF_LOG_INFO("Starting up the test project with USB logging");
    main_logs_init();
    main_click_init();
    while (true)
    {   

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
    

}

// int main(void)
// {
//     led_init_all();
//     pwm_led_init();

//     NRF_LOG_INFO("Starting up the test project with USB logging");
//     main_logs_init();
//     main_click_init();

//     int timing[LEDS_NUMBER];
//     id_to_led_blink(DEVICE_ID, timing, LEDS_NUMBER);
//     custom_blink_f = discrete_blink;

//     NRF_LOG_INFO("Starting to blink ...");
//     uint32_t num_ticks = ROUNDED_DIV(BLINK_DURATION + PAUSE_DURATION, PWM_PERIOD_MS);
//     blink_hold = 0;
//     intensity_hold = 1;
//     intensity_cur = num_ticks / 4;

//     while(true)
//     {
        
//         for(int led = 0; led < LEDS_NUMBER; led++)
//         {
//             for(int led_blinks = 0; led_blinks < timing[led]; led_blinks++)
//             {
//                 NRF_LOG_INFO("Blinking led %d, %d / %d", led, led_blinks + 1, timing[led]);
//                 for(uint32_t tick = 0; tick < num_ticks; tick++)
//                 {
//                     custom_blink_wrapper(led, tick, num_ticks);

//                     LOG_BACKEND_USB_PROCESS();
//                     NRF_LOG_PROCESS();
//                 }
//             }
//         }
        
//     }
    
// }


/**
 *@}
 **/
