#include "main_logs.h"

void main_logs_init()
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    
    if(ret != NRFX_SUCCESS)
    {
        blink_error();
    }

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}
