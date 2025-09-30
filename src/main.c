#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Boards Init Header
#include "board.h"
#include "i2c.h"
#include "log.h"
#include "i2c_proto.h"

static const char *TAG = "MAIN";

void app_main() 
{
    esp_err_t err = ESP_OK;
    err = board_init();

    if (err != ESP_OK) 
    {
        esp_log_level_set("*", ESP_LOG_ERROR);
        log_message(LOG_LEVEL_INFO, TAG, "Board initialization failed: %s", esp_err_to_name(err));
        return;
    }

    while (1) 
    {
        #if defined(BOARD_MASTER)
        i2c_frame_t frame = { 0 };
        int addr = i2c_hasSlaveData();
        if (addr) {
            if (i2c_recv_frame(SLAVE1_ADDR, &frame, pdMS_TO_TICKS(100)) == ESP_OK) {
                log_message(LOG_LEVEL_INFO, TAG, "RX from slave 0x%02X: %s", addr, (char*)frame.payload);
            }
        }
        i2c_frame_t cmd = {
            .header.cmd = CMD_PING
        };
        log_message(LOG_LEVEL_INFO, TAG, "Send PING to slave1");
        i2c_send_frame(SLAVE1_ADDR, &cmd, pdMS_TO_TICKS(100));
        #endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}