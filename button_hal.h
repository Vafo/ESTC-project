#ifndef BUTTON_HAL_H
#define BUTTON_HAL_H

#include <stdint.h>
#include "boards.h"


#define BTN_GPIO    NRF_GPIO_PIN_MAP(1,6)
#define BTN_PULL    NRF_GPIO_PIN_PULLUP     

void button_init(void);
uint32_t button_pressed(void);

#endif