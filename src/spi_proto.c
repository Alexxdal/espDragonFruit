#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

#include "log.h"
#include "spi_proto.h"

DMA_ATTR static proto_frame_t tx_frame = { 0 };
DMA_ATTR static proto_frame_t rx_frame = { 0 };
static QueueHandle_t tx_frame_queue;

static const char *TAG = "SPI_PROTO";

esp_err_t spi_proto_init(void)
{
    tx_frame_queue = xQueueCreate(FRAME_QUEUE_SIZE, sizeof(proto_frame_t));
    if(tx_frame_queue == NULL)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize tx frame queue!");
        return ESP_ERR_NO_MEM;
    }
#if defined(BOARD_MASTER)
    xTaskCreate(proto_master_task, "proto_master_task", 4096, NULL, 5, NULL);
#else
    xTaskCreate(proto_slave_task, "proto_slave_task", 4096, NULL, 5, NULL);
#endif
    log_message(LOG_LEVEL_INFO, TAG, "SPI Protocol manager initialized");
    return ESP_OK; 
}

static proto_frame_t *handle_frame(const proto_frame_t *frame)
{
    static proto_frame_t response = { 0 };
    log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d from slave=%d", frame->header.cmd, frame->header.addr);

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

esp_err_t proto_send_frame(int slave_addr, proto_frame_t *frame)
{
    frame->header.addr = slave_addr;
    if(xQueueSend(tx_frame_queue, frame, 50) != pdTRUE)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "Failed to add frame to queue");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

#if defined(BOARD_MASTER)
void proto_master_task(void *arg)
{
    (void)arg;
    while(1)
    {
        /* Check if there are some message in the queue */
        if(xQueueReceive(tx_frame_queue, &tx_frame, portMAX_DELAY) == pdFALSE)
            continue;

        for(int i = 0; i < 3; i ++)
        {
            esp_err_t e = spi_submit(i, (uint8_t *)&tx_frame, (uint8_t *)&rx_frame, 10);
            if(e == ESP_OK)
            {
                if(rx_frame.header.cmd != 0xffff && rx_frame.header.cmd != 0x0)
                {
                    handle_frame(&rx_frame);
                }
            }
        }     
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
void proto_slave_task(void *arg)
{
    (void)arg;
    while (1)
    {
        /* Check if there are some message in the queue */
        if(xQueueReceive(tx_frame_queue, &tx_frame, 10) == pdTRUE)
        {
            tx_frame.header.addr = SLAVE_ADDR;
        }
        else {
            tx_frame.header.addr = 0;
            tx_frame.header.cmd = 0;
            tx_frame.header.len = 0;
        }

        esp_err_t e = spi_submit(SLAVE_ADDR, (uint8_t *)&tx_frame, (uint8_t *)&rx_frame, portMAX_DELAY);
        if(e == ESP_OK)
        {
            if(rx_frame.header.addr == SLAVE_ADDR)
            {
                proto_frame_t *response = handle_frame(&rx_frame);
                if(response != NULL)
                {
                    xQueueSend(tx_frame_queue, response, 10);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
#endif /* any SLAVE */