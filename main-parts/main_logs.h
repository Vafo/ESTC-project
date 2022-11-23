#ifndef MAIN_LOGS_H
#define MAIN_LOGS_H

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#include "main_blinking.h"


/**
 * @brief Function for initializing logs of main app.
 */
void main_logs_init();

#endif