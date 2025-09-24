#include <assert.h>
#include <nvs_flash.h>
#include "esp_log.h"
#include "esp_check.h"
#include "board.h"
#include "log.h"

static const char *TAG = "BOARD";

__attribute__((weak)) esp_err_t master_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Master board initialized.\n");
    return ESP_OK;
}

__attribute__((weak)) esp_err_t slave_one_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave One board initialized.\n");
    return ESP_OK;
}

__attribute__((weak)) esp_err_t slave_two_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave Two board initialized.\n");
    return ESP_OK;
}

__attribute__((weak)) esp_err_t slave_three_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave Three board initialized.\n");
    return ESP_OK;
}


esp_err_t board_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "nvs_flash_erase");
        err = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(err, TAG, "nvs_flash_init");

#if defined(BOARD_MASTER)
    return master_init();

#elif defined(BOARD_SLAVE1)
    return slave_one_init();

#elif defined(BOARD_SLAVE2)
    return slave_two_init();

#elif defined(BOARD_SLAVE3)
    return slave_three_init();

#else
#   error "Define exactly one of: BOARD_MASTER, BOARD_SLAVE1, BOARD_SLAVE2, BOARD_SLAVE3"
#endif
}
