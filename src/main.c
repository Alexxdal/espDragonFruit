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

    while (1) 
    {
        vTaskDelay(portMAX_DELAY);
    }
}