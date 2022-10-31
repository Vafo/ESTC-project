/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "blink_hal.h"
#include "button_hal.h"

#define DEVICE_ID 7199
#define BLINK_DURATION 500 // ms
#define PAUSE_DURATION 500 

void id_to_led_blink(int num, int (*led_blink)[LEDS_NUMBER])
{
    int i;
    for(i = LEDS_NUMBER - 1; i >= 0 ; i--)
    {
        (*led_blink)[i] = num % 10;
        num /= 10;
    }
}

/**
 * @brief Function for application main entry.
 */
 #define BTN_PRESS
// #define BTN_HOLD

#if defined(BTN_PRESS)

int main(void)
{
    // Configure board. 
    led_init();
    button_init();

    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, &timing);

    // Toggle LEDs.
    int i = 0,
        j = 0;
    while (true)
    {
        while(button_pressed())
        {
            led_invert(i);
            nrf_delay_ms(BLINK_DURATION);
            led_invert(i);
            nrf_delay_ms(PAUSE_DURATION);
            j++;
            if(j >= timing[i])
            {
                j = 0;
                i++;
            }
            if(i >= LEDS_NUMBER)
            {
                i = 0;
            }
        }
        
    }
    
}

#elif defined(BTN_HOLD)

#include "nrfx_gpiote.h"


nrfx_gpiote_in_config_t  conf_release = {
    .sense = NRF_GPIOTE_POLARITY_LOTOHI,    // Release
    .pull = NRF_GPIO_PIN_PULLUP,
    .is_watcher = false,
    .hi_accuracy = 1,
    .skip_gpio_setup = false
};

nrfx_gpiote_in_config_t  conf_press = {
    .sense = NRF_GPIOTE_POLARITY_HITOLO,
    .pull = NRF_GPIO_PIN_PULLUP,
    .is_watcher = false,
    .hi_accuracy = 1,
    .skip_gpio_setup = false
};


void on_release(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void on_press(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    nrfx_gpiote_in_uninit(pin);
    nrfx_gpiote_in_init(pin, &conf_release, on_release);
    nrfx_gpiote_in_event_enable(pin, true);
    

    /*
    if(action == NRF_GPIOTE_POLARITY_HITOLO)
        led_on(LED_1_IDX);
    
    if(action == NRF_GPIOTE_POLARITY_LOTOHI)
        led_off(LED_1_IDX);
    */
}



void on_release(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    nrfx_gpiote_in_uninit(pin);
    nrfx_gpiote_in_init(pin, &conf_press, on_press);
    nrfx_gpiote_in_event_enable(pin, true);
    
    led_on(LED_2BLUE_IDX);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    while(!NVIC_GetPendingIRQ(GPIOTE_IRQn))
        ;

    led_off(LED_2BLUE_IDX);
    
}

int main(void)
{
    
    led_init();
    nrfx_gpiote_init();
    nrfx_err_t err_code1 = nrfx_gpiote_in_init(BUTTON_1, &conf_release, on_release);
    // Configure board. 

    nrfx_gpiote_in_event_enable(BUTTON_1, true);

    if(err_code1 != NRFX_SUCCESS)
    {
        while (true)
        {
            led_invert(LED_2RED_IDX);
            nrf_delay_ms(250);
            led_invert(LED_2RED_IDX);
            nrf_delay_ms(250);
        }
        
    }
    /*
    while(true)
    {
        
        led_invert(LED_2GREEN_IDX);
        nrf_delay_ms(BLINK_DURATION);
        led_invert(LED_2GREEN_IDX);
        nrf_delay_ms(PAUSE_DURATION);
        
    }
    */

    
    int timing[LEDS_NUMBER];
    id_to_led_blink(DEVICE_ID, &timing);

    // Toggle LEDs.
    on_release(BUTTON_1, NRF_GPIOTE_POLARITY_LOTOHI);
    while(true)
    {
        for(int i = 0; i < LEDS_NUMBER; i++)
        {
            for(int j = 0; j < timing[i]; j++)
            {
                led_invert(i);
                nrf_delay_ms(BLINK_DURATION);
                led_invert(i);
                nrf_delay_ms(PAUSE_DURATION);
            }
        }
    }
    
}

#endif

/**
 *@}
 **/
