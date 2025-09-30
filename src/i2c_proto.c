#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"

#include "log.h"
#include "i2c.h"
#include "i2c_proto.h"

static const char *TAG = "I2C_PROTO";

#if defined(BOARD_MASTER)
esp_err_t i2c_send_frame(uint8_t addr, i2c_frame_t *frame, TickType_t to)
{
    /* Check len */
    if(frame->header.len > I2C_MAX_PAYLOAD)
        return ESP_ERR_INVALID_SIZE;
    /* Check CRC */
    //Skip for now
    esp_err_t err = i2c_write_to(addr, (const uint8_t *)frame, sizeof(i2c_header_t) + frame->header.len, to);
    if (err != ESP_OK) 
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to send i2c frame cmd=%02X err=%s", frame->header.cmd, esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

esp_err_t i2c_recv_frame(uint8_t addr, i2c_frame_t *frame, TickType_t to)
{
    esp_err_t err = i2c_read_from(addr, (uint8_t *)&frame->header, sizeof(i2c_header_t), to);
    if(err != ESP_OK)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to read i2c frame.. err=%s", esp_err_to_name(err));
        return err;
    }
    if(frame->header.len > 0)
    {
        err = i2c_read_from(addr, frame->payload, frame->header.len, to);
        if(err != ESP_OK)
        {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to read i2c frame.. err=%s", esp_err_to_name(err));
            return err;
        }
    }
    /* CRC Check */
    return ESP_OK;
}
#endif 
static i2c_frame_t * handle_command(const i2c_frame_t *frame)
{
    static i2c_frame_t response = { 0 };
    switch (frame->header.cmd) {
        case CMD_PING: {
            response.header.cmd = CMD_PONG;
            response.header.len = 0;
            response.header.crc = 0;
            break;
        }
        case CMD_WIFI_ON: {
            // TODO: accendi davvero il Wi-Fi qui (non bloccare!)
            //make_resp(CMD_WIFI_ON, NULL, 0);
            break;
        }
        case CMD_WIFI_SCAN: {
            // TODO: lancia una scansione vera in task separato; qui demo
            //static const char demo[] = "ssid:demo,rssi:-42";
            //make_resp(CMD_WIFI_SCAN, (const uint8_t*)demo, sizeof(demo));
            break;
        }
        default: {
            //make_resp(0x00, NULL, 0);  // comando sconosciuto
            break;
        }
    }

    return &response;
}

#if defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)

void i2c_slave_task(void *arg)
{
    (void)arg;
    i2c_frame_t frame = { 0 };
    i2c_drdy_clear();

    while (1) 
    {
        esp_err_t n = i2c_slave_read_buffer(I2C_PORT, (uint8_t *)&frame, sizeof(i2c_frame_t), pdMS_TO_TICKS(50));
        if (n <= 0) 
        {
            continue;
        }
        /* Incomplete Frame */
        if(n != frame.header.len + sizeof(i2c_header_t))
        {
            continue;
        }
        
        i2c_drdy_clear();
        i2c_frame_t *response = handle_command(&frame);
        if(!response)
            continue;

        n = i2c_slave_write_buffer(I2C_PORT, (const uint8_t *)response, sizeof(i2c_header_t) + response->header.len, pdMS_TO_TICKS(50));
        if (n <= 0) 
        {
            log_message(LOG_LEVEL_ERROR, TAG, "slave_write_buffer failed");
            continue;
        }

        i2c_drdy_ready();
    }
}
#endif /* any SLAVE */