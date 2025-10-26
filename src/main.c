#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Boards Init Header
#include "board.h"
#include "log.h"
#include "spi_proto.h"
#include "commandMng.h"

static const char *TAG = "MAIN";

void app_main() 
{
    esp_log_level_set("*", ESP_LOG_ERROR);

    #if defined(BOARD_MASTER)
    /* Set Slave CS to 1 */
    gpio_set_direction(SPI_S1_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(SPI_S2_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(SPI_S3_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(SPI_S1_CS, 1);
    gpio_set_level(SPI_S2_CS, 1);
    gpio_set_level(SPI_S3_CS, 1);
    #endif

    esp_err_t err = ESP_OK;
    err = board_init();

    if (err != ESP_OK) 
    {
        log_message(LOG_LEVEL_INFO, TAG, "Board initialization failed: %s", esp_err_to_name(err));
        return;
    }

    #if defined(BOARD_MASTER)
    sta_config_t sta_config = {
        .ssid = "",
        .password = "",
        .pmf_capable = 1,
        .pmf_required = 0,
        .scan_method = WIFI_FAST_SCAN
    };
    CommandSetWifiConfig(ESPWROOM32, NULL, &sta_config, WIFI_MODE_STA);
    CommandSetWifiConfig(ESP32S3, NULL, &sta_config, WIFI_MODE_STA);
    CommandSetWifiConfig(ESP32C5, NULL, &sta_config, WIFI_MODE_STA);
    scan_config_t scan_config = {
        .channel = 0,
        .show_hidden = 1,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = 120,
    };
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for wifi to be ready
    CommandWifiScan(ESP32C5, &scan_config);
    CommandWifiScan(ESP32S3, &scan_config);
    CommandWifiScan(ESPWROOM32, &scan_config);
    wifi_scan(NULL);
    #endif

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}