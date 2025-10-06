#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

#include "log.h"
#include "i2c.h"
#include "i2c_proto.h"

static const char *TAG = "I2C_PROTO";
static QueueHandle_t rx_frame_queue = NULL;

esp_err_t i2c_proto_init(void)
{
    rx_frame_queue = xQueueCreate(CMD_QUEUE_SIZE, sizeof(i2c_frame_t));
    if(rx_frame_queue == NULL)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize rx frame queue!");
        return ESP_ERR_NO_MEM;
    }

#if defined(BOARD_MASTER)

    xTaskCreate(i2c_master_rx_task, "i2c_master_rx_task", 4096, NULL, 5, NULL);
    xTaskCreate(master_cmd_handel_task, "master_cmd_handel_task", 4096, NULL, 5, NULL);
#else
    xTaskCreate(i2c_slave_rx_task, "i2c_slave_rx_task", 4096, NULL, 5, NULL);
    xTaskCreate(slave_cmd_handle_task, "slave_cmd_handle_task", 4096, NULL, 5, NULL);
#endif

    log_message(LOG_LEVEL_INFO, TAG, "I2C Protocol manager initialized");
    return ESP_OK; 
}

static i2c_frame_t *handle_command(const i2c_frame_t *frame)
{
    static i2c_frame_t response = { 0 };

    log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d", frame->header.cmd);

    switch (frame->header.cmd) {
        case CMD_PING: {
            response.header.cmd = CMD_PONG;
            response.header.len = 0;
            response.header.crc = 0;
            return &response;
        }
        case CMD_WIFI_ON: {
            break;
        }
        case CMD_WIFI_SCAN: {
            break;
        }
        default: {
            break;
        }
    }

    return NULL;
}

#if defined(BOARD_MASTER)
esp_err_t i2c_send_frame(uint8_t addr, i2c_frame_t *frame, TickType_t to)
{
    if(frame == NULL)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "i2c_send_frame: frame is null!");
        return ESP_ERR_INVALID_ARG;
    }

    if(frame->header.len > (I2C_MAX_PAYLOAD - sizeof(i2c_header_t)))
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "i2c_send_frame: header len is greater than max payload size!");
        return ESP_ERR_INVALID_SIZE;
    }

    //TODO: CRC Check here

    esp_err_t err = i2c_write_to(addr, (const uint8_t *)frame, sizeof(i2c_header_t) + frame->header.len, to);
    if (err != ESP_OK) 
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "Failed to send i2c frame cmd=%02X err=%s", frame->header.cmd, esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

esp_err_t i2c_recv_frame(uint8_t addr, i2c_frame_t *frame, TickType_t to)
{
    if(frame == NULL)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "i2c_recv_frame: frame is null!");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = i2c_read_from(addr, (uint8_t *)frame, sizeof(i2c_header_t), to);
    if(err != ESP_OK)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "Failed to read i2c frame.. err=%s", esp_err_to_name(err));
        return err;
    }

    if(frame->header.len > 0 && frame->header.len < (I2C_MAX_PAYLOAD - sizeof(i2c_header_t)))
    {
        err = i2c_read_from(addr, (uint8_t *)frame->payload, frame->header.len, to);
        if(err != ESP_OK)
        {
            log_message(LOG_LEVEL_DEBUG, TAG, "Failed to read i2c frame.. err=%s", esp_err_to_name(err));
            return err;
        }
    }
    
    //TODO: CRC Check here

    return ESP_OK;
}

void i2c_master_rx_task(void *arg)
{
    (void)arg;
    int addr = 0;
    i2c_frame_t frame = { 0 };

    while(1)
    {
        addr = i2c_hasSlaveData();
        if (addr) 
        {
            if (i2c_recv_frame(addr, &frame, pdMS_TO_TICKS(50)) != ESP_OK) 
            {
                log_message(LOG_LEVEL_DEBUG, TAG, "Failed to read frame from slave n=%d", addr);
                continue;
            }

            if(xQueueSend(rx_frame_queue, &frame, pdMS_TO_TICKS(50)) != pdTRUE)
            {
                log_message(LOG_LEVEL_DEBUG, TAG, "Failed to add frame to queue");
                continue;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void master_cmd_handel_task(void *arg)
{
    (void)arg;
    i2c_frame_t rx_frame = { 0 };

    while(1)
    {
        if (xQueueReceive(rx_frame_queue, &rx_frame, portMAX_DELAY))
        {
            handle_command(&rx_frame);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#endif 

#if defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)

void i2c_slave_rx_task(void *arg)
{
    (void)arg;

    while (1)
    {
        i2c_frame_t frame = { 0 };

        int n = i2c_slave_read_buffer(I2C_PORT, (uint8_t *)&frame, sizeof(i2c_header_t), pdMS_TO_TICKS(50));
        if (n <= 0) 
        {
            continue;
        }
        
        if(n != sizeof(i2c_header_t))
        {
            log_message(LOG_LEVEL_DEBUG, TAG, "Failed to read frame header.");
            continue;
        }

        if(frame.header.len != 0)
        {
            n += i2c_slave_read_buffer(I2C_PORT, (uint8_t *)frame.payload, frame.header.len, pdMS_TO_TICKS(50));
        }

        if(n != frame.header.len + sizeof(i2c_header_t))
        {
            continue;
        }

        if(xQueueSend(rx_frame_queue, &frame, pdMS_TO_TICKS(50)) != pdTRUE)
        {
            log_message(LOG_LEVEL_DEBUG, TAG, "Failed to add frame to queue");
            continue;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void slave_cmd_handle_task(void *arg)
{
    (void)arg;
    int n = 0;

    while(1)
    {
        i2c_frame_t rx_frame = { 0 };

        if (xQueueReceive(rx_frame_queue, &rx_frame, portMAX_DELAY))
        {
            i2c_drdy_clear();
            i2c_frame_t *response = handle_command(&rx_frame);
            if(response == NULL)
            {
                continue;
            }
            
            n = i2c_slave_write_buffer(I2C_PORT, (const uint8_t *)response, sizeof(i2c_header_t) + response->header.len, pdMS_TO_TICKS(50));
            if (n <= 0) 
            {
                log_message(LOG_LEVEL_DEBUG, TAG, "i2c_slave_write_buffer failed");
                continue;
            }
            i2c_drdy_ready();
            vTaskDelay(pdMS_TO_TICKS(1));
            i2c_drdy_clear();
        }
        //This delay maybe is not necessary because xQueueReceive should block until frame is received
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#endif /* any SLAVE */