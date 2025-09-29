#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Boards Init Header
#include "board.h"
#include "spi_protocol.h"
#include "log.h"

static const char *TAG = "APP";

void app_main() 
{
    esp_err_t err = ESP_OK;
    err = board_init();

    if (err != ESP_OK) 
    {
        esp_log_level_set("*", ESP_LOG_ERROR);
        log_message(LOG_LEVEL_ERROR, TAG, "Board initialization failed: %s", esp_err_to_name(err));
        return;
    }

    while (1) 
    {  
#if defined(BOARD_MASTER)
        int ep = 0;
        const char msg[] = "hello";
        while (1) 
        {
            ESP_LOGI(TAG, "PING → ep=%d", ep);
            spi_send_frame(ep, SPI_CMD_PING, (const uint8_t*)msg, sizeof(msg));
            ep = (ep + 1) % EP_MAX;
            vTaskDelay(pdMS_TO_TICKS(500));
        }
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
        
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}