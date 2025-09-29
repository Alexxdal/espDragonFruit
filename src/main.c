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
        int addr = i2c_hasSlaveData();
        if (addr) {
            uint8_t buf[64];
            uint8_t len = 0;
            if (i2c_recv_resp(addr, buf, sizeof(buf), &len, pdMS_TO_TICKS(200)) == ESP_OK) {
                buf[len] = 0; // termina stringa
                log_message(LOG_LEVEL_INFO, TAG, "RX from slave 0x%02X: %s", addr, (char*)buf);
            }
        }

        log_message(LOG_LEVEL_INFO, TAG, "Send PING to slave1");
        i2c_send_cmd(SLAVE1_ADDR, CMD_PING, (const uint8_t*)"hello", 5, pdMS_TO_TICKS(200));
        #endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}