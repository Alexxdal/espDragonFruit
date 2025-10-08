#include <assert.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "esp_check.h"
#include "board.h"
#include "spi_proto.h"
#include "wifi.h"
#include "log.h"
#include "netif.h"
#include "nvs_config.h"

#if defined(BOARD_MASTER)
#include "esp_spiffs.h"
#include "httpd.h"
#endif

static const char *TAG = "BOARD";
static board_status_t board_status = { 0 };

#if defined(BOARD_MASTER)
esp_err_t master_init()
{
    esp_err_t err = ESP_OK;

    err = network_interface_init();
    ESP_RETURN_ON_ERROR(err, TAG, "netif_init");

    err = wifi_init_apsta();
    ESP_RETURN_ON_ERROR(err, TAG, "wifi_init_apsta");

    err = wifi_start_softap();
    ESP_RETURN_ON_ERROR(err, TAG, "wifi_start_softap");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "spiffs",
        .max_files = 15,
        .format_if_mount_failed = false
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    err = httpd_server_start();
    ESP_RETURN_ON_ERROR(err, TAG, "httpd_server_start");

    log_message(LOG_LEVEL_INFO, TAG, "Master board initialized.\n");
    return err;
}
#elif defined(BOARD_SLAVE1)
esp_err_t slave_one_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave One board initialized.\n");
    return ESP_OK;
}
#elif defined(BOARD_SLAVE2)
esp_err_t slave_two_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave Two board initialized.\n");
    return ESP_OK;
}
#elif defined(BOARD_SLAVE3)
esp_err_t slave_three_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave Three board initialized.\n");
    return ESP_OK;
}
#endif

esp_err_t board_init(void)
{
    ESP_ERROR_CHECK(nvs_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    /* Get Chip info */
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    board_status.chip.cores = chip.cores;
    board_status.chip.features = chip.features;
    board_status.chip.model = chip.model;
    board_status.chip.revision = chip.revision;

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

    ESP_ERROR_CHECK(spi_init());
    ESP_ERROR_CHECK(spi_proto_init());

    return ESP_OK;
}

board_status_t *getBoardStatus(void)
{
    return &board_status;
}