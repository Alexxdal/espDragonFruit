#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Boards Init Header
#include "board.h"

void app_main() 
{
    esp_err_t err = ESP_OK;
    err = board_init();

    if (err != ESP_OK) 
    {
        esp_log_level_set("*", ESP_LOG_ERROR);
        ESP_LOGE("APP", "Board initialization failed: %s", esp_err_to_name(err));
        return;
    }

    // Main application loop
    while (1) 
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}