#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Boards Init Header
#include "board.h"
#include "log.h"
#include "spi_proto.h"
#include "cJSON.h"

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

    while (1) 
    {
        #if defined(BOARD_MASTER)
        proto_chip_info_t chip_info = {
            .header.cmd = CMD_CHIP_INFO
        };

        log_message(LOG_LEVEL_INFO, TAG, "Send PING to slave1");
        proto_send_frame(ESPWROOM32, &chip_info);

        vTaskDelay(pdMS_TO_TICKS(500));

        log_message(LOG_LEVEL_INFO, TAG, "Send PING to slave2");
        proto_send_frame(ESP32C5, &chip_info);

        vTaskDelay(pdMS_TO_TICKS(500));

        log_message(LOG_LEVEL_INFO, TAG, "Send PING to slave3");
        proto_send_frame(ESP32S3, &chip_info);
        #endif

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}