#include "nvmc_api.h"
#include "nrf_log.h"
#include "util/sdk_macros.h"

    // NRF_DFU_APP_DATA_AREA_SIZE
    // BOOTLOADER_START_ADDR

#define MAGIC_WORD 0x56781231
#define NVMC_API_GET_PAGE(pos) ( (pos) & (~(BOOTLOADER_SETTINGS_PAGE_SIZE - 1)) )


void nvmc_api_init(nvmc_state_t *nvmc_state)
{
    nvmc_state->cur_read_pos = nvmc_state->cur_write_pos = NVMC_API_APPLICATION_BEGIN;

    nvmc_api_word_t sign_word = NVMC_API_EMPTY_WORD_SLOT;

    nvmc_api_read_next_word(nvmc_state, &sign_word);

    nvmc_api_address_t initial_pos = nvmc_api_get_cur_read_pos(nvmc_state);
    if(sign_word == MAGIC_WORD)
    {
        NRF_LOG_INFO("Flash is not empty");   
        while(sign_word != NVMC_API_EMPTY_WORD_SLOT)
        {
            nvmc_api_read_next_word(nvmc_state, &sign_word);
        }
        nvmc_api_set_cur_write_pos(nvmc_state, nvmc_state->cur_read_pos - sizeof(nvmc_api_word_t));
        // nvmc_api_set_cur_read_pos(nvmc_state, initial_pos);
    }
    else
    {
        NRF_LOG_INFO("Flash is empty");
        nrfx_nvmc_page_erase(NVMC_API_GET_PAGE(nvmc_state->cur_read_pos));
        sign_word = MAGIC_WORD;
        nvmc_api_write_next_word(nvmc_state, &sign_word);
    }
    nvmc_api_set_cur_read_pos(nvmc_state, initial_pos);

}

nrfx_err_t nvmc_api_read_next_word(nvmc_state_t *nvmc_state, nvmc_api_word_t *dest)
{
    VERIFY_TRUE(dest != NULL, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(IS_VALID_POS(nvmc_api_get_cur_read_pos(nvmc_state)), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(IS_VALID_POS(nvmc_api_get_cur_read_pos(nvmc_state) + sizeof(nvmc_api_word_t) - 1), \
                NRF_ERROR_INVALID_LENGTH);

    NRF_LOG_INFO("Reading at %x", nvmc_state->cur_read_pos);
    *dest = *((nvmc_api_word_t *) nvmc_state->cur_read_pos);
    nvmc_state->cur_read_pos += sizeof(nvmc_api_word_t);

    return NRF_SUCCESS;
}

nrfx_err_t nvmc_api_write_next_word(nvmc_state_t *nvmc_state, nvmc_api_word_t *src)
{
    VERIFY_TRUE(src != NULL, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(IS_VALID_POS(nvmc_api_get_cur_write_pos(nvmc_state)), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(IS_VALID_POS(nvmc_api_get_cur_write_pos(nvmc_state) + sizeof(nvmc_api_word_t) - 1), \
                NRF_ERROR_INVALID_LENGTH);

    nvmc_api_wait_until_written();
    nrfx_nvmc_word_write(nvmc_state->cur_write_pos, *src);
    nvmc_state->cur_write_pos += sizeof(nvmc_api_word_t);

    return NRF_SUCCESS;
}

nrfx_err_t nvmc_api_read_next_n_bytes(nvmc_state_t *nvmc_state, nvmc_api_byte_t *dest, size_t num_bytes)
{
    nrfx_err_t retcode;
    retcode = nvmc_api_read_cur_n_bytes(nvmc_state, dest, num_bytes);

    if(retcode != NRF_SUCCESS)
    {
        return retcode;
    }

    nvmc_api_set_cur_read_pos(nvmc_state,  (nvmc_state->cur_read_pos + sizeof(nvmc_api_byte_t) * num_bytes) );
    return NRF_SUCCESS;
}


nrfx_err_t nvmc_api_read_cur_n_bytes(nvmc_state_t *nvmc_state, nvmc_api_byte_t *dest, size_t num_bytes)
{
    VERIFY_TRUE(dest != NULL, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(num_bytes >= 0, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(IS_VALID_POS(nvmc_api_get_cur_read_pos(nvmc_state)), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(IS_VALID_POS(nvmc_api_get_cur_read_pos(nvmc_state) + (sizeof(nvmc_api_byte_t) * num_bytes) - 1), \
                NRF_ERROR_INVALID_LENGTH);

    nvmc_api_byte_t *loc_begin = (nvmc_api_byte_t *) nvmc_state->cur_read_pos;
    nvmc_api_byte_t *loc_end = (nvmc_api_byte_t *) (nvmc_state->cur_read_pos + sizeof(nvmc_api_byte_t) * num_bytes);
    nvmc_api_byte_t *cur_loc;

    for(cur_loc = loc_begin; cur_loc < loc_end; cur_loc++)
    {
        *dest++ = *cur_loc;
    }

    return NRF_SUCCESS;
}

nrfx_err_t nvmc_api_write_next_n_bytes(nvmc_state_t *nvmc_state, nvmc_api_byte_t *src, size_t num_bytes)
{
    VERIFY_TRUE(src != NULL, NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(num_bytes >= 0, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(IS_VALID_POS(nvmc_api_get_cur_write_pos(nvmc_state)), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(nvmc_api_get_cur_write_pos(nvmc_state) + (sizeof(nvmc_api_byte_t) * num_bytes) - 1, \
                NRF_ERROR_INVALID_LENGTH);

    // nvmc_api_byte_t *loc_begin = (nvmc_api_byte_t *) nvmc_state->cur_write_pos;
    // nvmc_api_byte_t *loc_end = (nvmc_api_byte_t *) (nvmc_state->cur_write_pos + sizeof(nvmc_api_byte_t) * num_bytes);
    // nvmc_api_byte_t *cur_loc;
    

    nvmc_api_wait_until_written();
    nrfx_nvmc_bytes_write(nvmc_state->cur_write_pos, (const void *) src, num_bytes);

    nvmc_api_set_cur_write_pos(nvmc_state, (nvmc_state->cur_write_pos + sizeof(nvmc_api_byte_t) * num_bytes) );
    
    return NRF_SUCCESS;
}

void nvmc_api_set_cur_read_pos(nvmc_state_t *nvmc_state, nvmc_api_address_t read_pos)
{
    ASSERT_VALID_POS(read_pos);
    nvmc_state->cur_read_pos = read_pos;
}

nvmc_api_address_t nvmc_api_get_cur_read_pos(nvmc_state_t *nvmc_state)
{
    return nvmc_state->cur_read_pos;
}

void nvmc_api_set_cur_write_pos(nvmc_state_t *nvmc_state, nvmc_api_address_t write_pos)
{
    ASSERT_VALID_POS(write_pos);
    nvmc_state->cur_write_pos = write_pos;
}

nvmc_api_address_t nvmc_api_get_cur_write_pos(nvmc_state_t *nvmc_state)
{
    return nvmc_state->cur_write_pos;
}


void nvmc_api_wait_until_written()
{
    while( !nrfx_nvmc_write_done_check() )
        ;
}