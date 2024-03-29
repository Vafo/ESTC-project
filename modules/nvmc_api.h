#ifndef NVMC_API_H
#define NVMC_API_H

// #include "bootloader/dfu/nrf_dfu.h"
// #include "bootloader/nrf_bootloader_info.h"


// #include "nrfx_nvmc.h"
#include "nrf_fstorage.h"

#include "nrf_fstorage.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_fstorage_sd.h"
#include "nrf_soc.h"

#include "nrfx.h"

#define NVMC_API_EMPTY_WORD_SLOT ( (nvmc_api_word_t) 0xFFFFFFFF )
#define NVMC_API_EMPTY_BYTE_SLOT ( (nvmc_api_word_t) 0xFF)

#define BOOTLOADER_SETTINGS_PAGE_SIZE   ((1) << (12))

#define BOOTLOADER_START_ADDR (0xE0000)
#define NVMC_API_PAGE_NUMBER 3
#define NRF_DFU_APP_DATA_AREA_SIZE  (NVMC_API_PAGE_NUMBER * BOOTLOADER_SETTINGS_PAGE_SIZE)

#define NVMC_API_APPLICATION_BEGIN (BOOTLOADER_START_ADDR - NRF_DFU_APP_DATA_AREA_SIZE)
#define NVMC_API_APPLICATION_END (BOOTLOADER_START_ADDR)

#define IS_VALID_POS(pos) (NVMC_API_APPLICATION_BEGIN <= (pos) && (pos) < NVMC_API_APPLICATION_END)
#define IS_VALID_POS_IN_PAGE(pos, page_no) (NVMC_API_APPLICATION_BEGIN + (page_no) * BOOTLOADER_SETTINGS_PAGE_SIZE <= (pos) \
                                            && (pos) < NVMC_API_APPLICATION_BEGIN + ((page_no) + 1) * BOOTLOADER_SETTINGS_PAGE_SIZE)
#define ASSERT_VALID_POS(pos)   ASSERT( IS_VALID_POS(pos) )

typedef uint32_t nvmc_api_address_t;

typedef unsigned char nvmc_api_byte_t;
typedef unsigned int nvmc_api_word_t;

typedef struct 
{
    nvmc_api_address_t cur_write_pos;
    nvmc_api_address_t cur_read_pos;
    uint8_t page_no;
    uint8_t is_valid;
} nvmc_state_t;


nrfx_err_t nvmc_api_init(nvmc_state_t *nvmc_state, uint8_t page_no);

nrfx_err_t nvmc_api_read_next_word(nvmc_state_t *nvmc_state, nvmc_api_word_t *dest);
nrfx_err_t nvmc_api_write_next_word(nvmc_state_t *nvmc_state, nvmc_api_word_t *src);

nrfx_err_t nvmc_api_read_next_n_bytes(nvmc_state_t *nvmc_state, nvmc_api_byte_t *dest, size_t num_bytes);
nrfx_err_t nvmc_api_write_next_n_bytes(nvmc_state_t *nvmc_state, nvmc_api_byte_t *src, size_t num_bytes);

nrfx_err_t nvmc_api_read_cur_n_bytes(nvmc_state_t *nvmc_state, nvmc_api_byte_t *dest, size_t num_bytes);

void nvmc_api_set_cur_read_pos(nvmc_state_t *nvmc_state, nvmc_api_address_t read_pos);
nvmc_api_address_t nvmc_api_get_cur_read_pos(nvmc_state_t *nvmc_state);

void nvmc_api_set_cur_write_pos(nvmc_state_t *nvmc_state, nvmc_api_address_t write_pos);
nvmc_api_address_t nvmc_api_get_cur_write_pos(nvmc_state_t *nvmc_state);

void nvmc_api_wait_until_written();

#endif