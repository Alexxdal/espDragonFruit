#include <assert.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "esp_check.h"
#include "board.h"
#include "spi.h"
#include "wifi.h"
#include "log.h"
#include "netif.h"
#include "nvs_config.h"

static const char *TAG = "BOARD";

__attribute__((weak)) esp_err_t master_init()
{
    esp_err_t err = ESP_OK;

    err = network_interface_init();
    ESP_RETURN_ON_ERROR(err, TAG, "netif_init");

    err = wifi_init_apsta();
    ESP_RETURN_ON_ERROR(err, TAG, "wifi_init_apsta");

    err = wifi_start_softap();
    ESP_RETURN_ON_ERROR(err, TAG, "wifi_start_softap");

    log_message(LOG_LEVEL_INFO, TAG, "Master board initialized.\n");
    return err;
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
    esp_err_t err = nvs_init();
    ESP_RETURN_ON_ERROR(err, TAG, "nvs_init");

    err = esp_event_loop_create_default();
    ESP_RETURN_ON_ERROR(err, TAG, "esp_event_loop_create_default");

    err = spi_init();
    ESP_RETURN_ON_ERROR(err, TAG, "spi_init");

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
