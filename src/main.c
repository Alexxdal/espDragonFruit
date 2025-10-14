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
    static ap_config_t ap = {
        .ssid = "ESPWROOM32",
        .password = "espDragonFruit",
        .channel = 1,
        .authmode = WIFI_AUTH_WPA2_PSK,
        .beacon_interval = 300,
        .max_connection = 5,
        .pmf_required = false,
        .pmf_capable = false
    };
    CommandSetWifiConfig(ESPWROOM32, &ap, NULL, WIFI_MODE_AP);

    strcpy((char *)ap.ssid, "ESP32C5");
    ap.channel = 6;
    CommandSetWifiConfig(ESP32C5, &ap, NULL, WIFI_MODE_AP);

    strcpy((char *)ap.ssid, "ESP32S3");
    ap.channel = 12;
    CommandSetWifiConfig(ESP32S3, &ap, NULL, WIFI_MODE_AP);
    #endif

    while (1) 
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}