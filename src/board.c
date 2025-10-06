#include <assert.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "esp_check.h"
#include "board.h"
#include "i2c_proto.h"
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
    ESP_ERROR_CHECK(nvs_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

#if defined(BOARD_MASTER)
    ESP_ERROR_CHECK(master_init());

#elif defined(BOARD_SLAVE1)
    ESP_ERROR_CHECK(slave_one_init());

#elif defined(BOARD_SLAVE2)
    ESP_ERROR_CHECK(slave_two_init());

#elif defined(BOARD_SLAVE3)
    ESP_ERROR_CHECK(slave_three_init());

#else
#   error "Define exactly one of: BOARD_MASTER, BOARD_SLAVE1, BOARD_SLAVE2, BOARD_SLAVE3"
#endif

    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(i2c_proto_init());

    return ESP_OK;
}
