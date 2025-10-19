#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
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
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_err_t err = ESP_OK;
    err = board_init();

    if (err != ESP_OK) 
    {
        log_message(LOG_LEVEL_INFO, TAG, "Board initialization failed: %s", esp_err_to_name(err));
        return;
    }

    /* Test start AP on slaves */
    #if defined(BOARD_MASTER)
    static sta_config_t sta_no_pmf = {
        .ssid = "TestSSID",
        .password = "TestPassword",
        .scan_method = WIFI_FAST_SCAN,
        .bssid_set = 0,
        .channel = 0,
        .listen_interval = 0,
        .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
        .pmf_capable = true,
        .pmf_required = false
    };

    static sta_config_t sta_yes_pmf = {
        .ssid = "TestSSID",
        .password = "TestPassword",
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .bssid_set = 0,
        .channel = 0,
        .listen_interval = 0,
        .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
        .pmf_capable = true,
        .pmf_required = true
    };
    
    CommandSetWifiConfig(ESPWROOM32, NULL, &sta_no_pmf, WIFI_MODE_STA);
    CommandSetWifiConfig(ESP32C5, NULL, &sta_yes_pmf, WIFI_MODE_STA);
    CommandSetWifiConfig(ESP32S3, NULL, &sta_yes_pmf, WIFI_MODE_STA);
    #endif

    while (1) 
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}