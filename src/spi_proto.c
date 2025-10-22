#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

#include "log.h"
#include "spi_proto.h"
#include "commandMng.h"

/* Statistics */
static uint32_t frame_sent;
static uint32_t frame_received;
static uint32_t frame_crc_error;

DMA_ATTR static proto_frame_t tx_frame;
DMA_ATTR static proto_frame_t rx_frame;
static QueueHandle_t tx_frame_queue;

static const char *TAG = "SPI_PROTO";

static uint8_t crc8_atm(const uint8_t *data, size_t len) {
    const uint8_t poly = 0x07;
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; ++i) 
    {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) 
        {
            if (crc & 0x80) 
                crc = (uint8_t)((crc << 1) ^ poly);
            else             
                crc <<= 1;
        }
    }
    return crc;
}

esp_err_t spi_proto_init(void)
{
    frame_sent = 0;
    frame_received = 0;
    frame_crc_error = 0;
    memset(&tx_frame, 0, sizeof(tx_frame));
    memset(&rx_frame, 0, sizeof(rx_frame));

    esp_err_t err = spi_init();
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize SPI Bus. err=%s", esp_err_to_name(err));
        return err;
    }

    tx_frame_queue = xQueueCreate(FRAME_QUEUE_SIZE, sizeof(proto_frame_t));
    if(tx_frame_queue == NULL)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize tx frame queue!");
        return ESP_ERR_NO_MEM;
    }
#if defined(BOARD_MASTER)
    xTaskCreate(proto_master_task, "proto_master_task", 4096, NULL, 5, NULL);
    xTaskCreate(proto_master_polling_task, "proto_master_polling_task", 4096, NULL, 6, NULL);
#else
    xTaskCreate(proto_slave_task, "proto_slave_task", 4096, NULL, 5, NULL);
#endif
    log_message(LOG_LEVEL_INFO, TAG, "SPI Protocol initialized");
    board_status_t *status = getBoardStatus();
    status->spi_status = true;
    return ESP_OK; 
}

esp_err_t proto_send_frame(int slave_addr, void *frame)
{
    proto_frame_t *in = (proto_frame_t *)frame;

    if(in->header.len > PROTO_MAX_PAYLOAD)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Frame len (%d) is greater than the max allowed", in->header.len);
        return ESP_ERR_INVALID_ARG;
    }

    in->header.addr = slave_addr;
    in->header.crc  = crc8_atm(((const uint8_t *)in) + 1, (sizeof(proto_header_t) - 1) + in->header.len);

    if(xQueueSend(tx_frame_queue, in, 100) != pdTRUE)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "Failed to add frame to queue");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

#if defined(BOARD_MASTER)
void proto_master_polling_task(void *arg)
{
    (void)arg;
    static proto_board_status_t poll_frame;
    memset(&poll_frame, 0, sizeof(poll_frame));
    poll_frame.header.cmd = CMD_BOARD_STATUS;
    poll_frame.header.len = 0;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_err_t err = proto_send_frame(BROADCAST_ADDR, &poll_frame);
        if(err != ESP_OK)
        {
            log_message(LOG_LEVEL_DEBUG, TAG, "Failed to send poll frame.");
        }
    }
}

void proto_master_task(void *arg)
{
    (void)arg;
    while(1)
    {
        /* Check if there are some message in the queue, if there are no message send poll */
        if(xQueueReceive(tx_frame_queue, &tx_frame, portMAX_DELAY) == pdTRUE)
        {
            int dst = tx_frame.header.addr;
            if (dst >= ESPWROOM32 && dst <= ESP32S3) 
            {
                int idx = dst - 1;
                esp_err_t e = spi_submit(idx, (uint8_t *)&tx_frame, (uint8_t *)&rx_frame, 0); // Master has no tick to wait
                if (e == ESP_OK) {
                    frame_sent++;
                    if (rx_frame.header.cmd != 0xffff && rx_frame.header.cmd != 0x0) {
                        uint8_t crc = crc8_atm(((const uint8_t *)&rx_frame) + 1, (sizeof(proto_header_t) - 1) + rx_frame.header.len);
                        if (rx_frame.header.crc == crc) {
                            frame_received++;
                            handle_frame_master(&rx_frame);
                        } else {
                            frame_crc_error++;
                            log_message(LOG_LEVEL_DEBUG, TAG, "Frame with CRC Error: %d from slave %d cmd=%02X len=%d.", frame_crc_error, rx_frame.header.addr, rx_frame.header.crc, rx_frame.header.len);
                        }
                    }
                }
            }
            else if (dst == BROADCAST_ADDR) 
            {
                for (int i = 0; i < 3; i++) {
                    esp_err_t e = spi_submit(i, (uint8_t *)&tx_frame, (uint8_t *)&rx_frame, 0); // Master has no tick to wait
                    if (e == ESP_OK) {
                        frame_sent++;
                        if (rx_frame.header.cmd != 0xffff && rx_frame.header.cmd != 0x0) {
                            uint8_t crc = crc8_atm(((const uint8_t *)&rx_frame) + 1, (sizeof(proto_header_t) - 1) + rx_frame.header.len);
                            if (rx_frame.header.crc == crc) {
                                frame_received++;
                                handle_frame_master(&rx_frame);
                            } else {
                                frame_crc_error++;
                                log_message(LOG_LEVEL_DEBUG, TAG, "Frame with CRC Error: %d from slave %d cmd=%02X len=%d.", frame_crc_error, rx_frame.header.addr, rx_frame.header.crc, rx_frame.header.len);
                            }
                        }
                    }
                }
            }
        }
    }
}
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
void proto_slave_task(void *arg)
{
    (void)arg;
    memset(&tx_frame, 0, sizeof(tx_frame));

    while (1)
    {
        /* Check if there are some message in the queue */
        if (xQueueReceive(tx_frame_queue, &tx_frame, 0) == pdTRUE) {
            // niente altro: tx_frame ora contiene il frame completo da inviare
        }

        esp_err_t e = spi_submit(SLAVE_ADDR, (uint8_t *)&tx_frame, (uint8_t *)&rx_frame, portMAX_DELAY);
        if(e == ESP_OK)
        {
            frame_sent++;
            if(rx_frame.header.addr == SLAVE_ADDR || rx_frame.header.addr == BROADCAST_ADDR)
            {
                uint8_t crc = crc8_atm((const uint8_t *)&rx_frame + 1, (sizeof(proto_header_t) - 1) + rx_frame.header.len);
                if(rx_frame.header.crc == crc)
                {
                    frame_received++;
                    proto_frame_t *response = handle_frame_slave(&rx_frame);
                    if(response != NULL)
                    {
                        proto_send_frame(SLAVE_ADDR, response);
                    }
                }
                else {
                    frame_crc_error++;
                    log_message(LOG_LEVEL_DEBUG, TAG, "Frame with CRC Error: %d addr=%d cmd=%02X len=%d.", frame_crc_error, rx_frame.header.addr, rx_frame.header.cmd, rx_frame.header.len);
                }
            }
        }
    }
}
#endif /* any SLAVE */