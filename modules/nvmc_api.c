#include "nvmc_api.h"
#include "nrf_log.h"
#include "sdk_macros.h"

    // NRF_DFU_APP_DATA_AREA_SIZE
    // BOOTLOADER_START_ADDR

#define MAGIC_WORD 0x56781231
#define NVMC_API_GET_PAGE(pos) ( (pos) & (~(BOOTLOADER_SETTINGS_PAGE_SIZE - 1)) )

static uint8_t m_fstorage_init = 0;

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr = NVMC_API_APPLICATION_BEGIN,
    .end_addr   = NVMC_API_APPLICATION_END,
};

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}

nrfx_err_t nvmc_api_init(nvmc_state_t *nvmc_state, uint8_t page_no)
{
    VERIFY_TRUE(0 <= page_no && page_no < NVMC_API_PAGE_NUMBER, \
                NRF_ERROR_INVALID_PARAM);
    ret_code_t ret;
    if(!m_fstorage_init)
    {
        m_fstorage_init = 1;
        ret = nrf_fstorage_init(&fstorage, &nrf_fstorage_sd, NULL);
        APP_ERROR_CHECK(ret);
    }

    nvmc_state->cur_read_pos = nvmc_state->cur_write_pos = NVMC_API_APPLICATION_BEGIN + page_no * BOOTLOADER_SETTINGS_PAGE_SIZE;
    nvmc_state->page_no = page_no;

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
        // nrfx_nvmc_page_erase(NVMC_API_GET_PAGE(nvmc_state->cur_read_pos));
        nrf_fstorage_erase(&fstorage, NVMC_API_GET_PAGE(nvmc_state->cur_read_pos), 1, NULL);
        sign_word = MAGIC_WORD;
        nvmc_api_write_next_word(nvmc_state, &sign_word);
    }
    nvmc_api_set_cur_read_pos(nvmc_state, initial_pos);

    return NRF_SUCCESS;
}

nrfx_err_t nvmc_api_read_next_word(nvmc_state_t *nvmc_state, nvmc_api_word_t *dest)
{
    VERIFY_TRUE(dest != NULL, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_read_pos(nvmc_state), nvmc_state->page_no), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_read_pos(nvmc_state) + sizeof(nvmc_api_word_t) - 1, nvmc_state->page_no), \
                NRF_ERROR_INVALID_LENGTH);

    NRF_LOG_INFO("Reading at %x", nvmc_state->cur_read_pos);
    // *dest = *((nvmc_api_word_t *) nvmc_state->cur_read_pos);
    nrf_fstorage_read(&fstorage, nvmc_state->cur_read_pos, dest, sizeof(nvmc_api_word_t));
    nvmc_state->cur_read_pos += sizeof(nvmc_api_word_t);

    return NRF_SUCCESS;
}

nrfx_err_t nvmc_api_write_next_word(nvmc_state_t *nvmc_state, nvmc_api_word_t *src)
{
    VERIFY_TRUE(src != NULL, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_write_pos(nvmc_state), nvmc_state->page_no), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_write_pos(nvmc_state) + sizeof(nvmc_api_word_t) - 1, nvmc_state->page_no), \
                NRF_ERROR_INVALID_LENGTH);

    nvmc_api_wait_until_written();
    // nrfx_nvmc_word_write(nvmc_state->cur_write_pos, *src);
    nrf_fstorage_write(&fstorage, nvmc_state->cur_write_pos, src, sizeof(nvmc_api_word_t), NULL);
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
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_read_pos(nvmc_state), nvmc_state->page_no), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_read_pos(nvmc_state) + (sizeof(nvmc_api_byte_t) * num_bytes) - 1, nvmc_state->page_no), \
                NRF_ERROR_INVALID_LENGTH);

    // nvmc_api_byte_t *loc_begin = (nvmc_api_byte_t *) nvmc_state->cur_read_pos;
    // nvmc_api_byte_t *loc_end = (nvmc_api_byte_t *) (nvmc_state->cur_read_pos + sizeof(nvmc_api_byte_t) * num_bytes);
    // nvmc_api_byte_t *cur_loc;
    nrf_fstorage_read(&fstorage, nvmc_state->cur_read_pos, dest, num_bytes);
    // for(cur_loc = loc_begin; cur_loc < loc_end; cur_loc++)
    // {
    //     *dest++ = *cur_loc;
    // }

    return NRF_SUCCESS;
}

nrfx_err_t nvmc_api_write_next_n_bytes(nvmc_state_t *nvmc_state, nvmc_api_byte_t *src, size_t num_bytes)
{
    VERIFY_TRUE(src != NULL, NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(num_bytes >= 0, NRF_ERROR_INVALID_PARAM);
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_write_pos(nvmc_state), nvmc_state->page_no), \
                NRF_ERROR_INVALID_STATE);
    VERIFY_TRUE(IS_VALID_POS_IN_PAGE(nvmc_api_get_cur_write_pos(nvmc_state) + (sizeof(nvmc_api_byte_t) * num_bytes) - 1, nvmc_state->page_no), \
                NRF_ERROR_INVALID_LENGTH);

    // nvmc_api_byte_t *loc_begin = (nvmc_api_byte_t *) nvmc_state->cur_write_pos;
    // nvmc_api_byte_t *loc_end = (nvmc_api_byte_t *) (nvmc_state->cur_write_pos + sizeof(nvmc_api_byte_t) * num_bytes);
    // nvmc_api_byte_t *cur_loc;
    

    nvmc_api_wait_until_written();
    // nrfx_nvmc_bytes_write(nvmc_state->cur_write_pos, (const void *) src, num_bytes);
    nrf_fstorage_write(&fstorage, nvmc_state->cur_write_pos, src, num_bytes, NULL);

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
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(&fstorage))
    {
        sd_app_evt_wait();
    }
}