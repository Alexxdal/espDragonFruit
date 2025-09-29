#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "spi.h"
#include "spi_protocol.h"

static const char *TAG = "SPI_PROTO";

static TaskHandle_t spi_rx_task_handle = NULL;
static uint8_t s_rx[EP_MAX][sizeof(spi_frame_t)];
static uint8_t s_tx[EP_MAX][sizeof(spi_frame_t)];

uint16_t spi_crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001; else crc >>= 1;
        }
    }
    return crc;
}

static esp_err_t arm_read(int ep)
{
    memset(s_tx[ep], 0, sizeof(spi_frame_t));
    return spi_submit(ep, s_tx[ep], sizeof(spi_frame_t), s_rx[ep], sizeof(spi_frame_t));
}

// --------- TX da Master verso Slave ---------
esp_err_t spi_send_frame(int ep, uint8_t command, const uint8_t *payload, uint16_t length)
{
    if (length > SPI_MAX_PAYLOAD_SIZE) return ESP_ERR_INVALID_SIZE;

    size_t frame_len = sizeof(frame_header_t) + length;
    uint8_t *buf = heap_caps_malloc(frame_len, MALLOC_CAP_DMA);
    if (!buf) return ESP_ERR_NO_MEM;

    frame_header_t *h = (frame_header_t*)buf;
    h->start_byte = SPI_START_BYTE;
    h->command    = command;
    h->length     = length;
    if (length && payload) memcpy(buf + sizeof(frame_header_t), payload, length);

    h->checksum = spi_crc16(buf, sizeof(frame_header_t) - sizeof(uint16_t) + length);

    esp_err_t err = spi_submit(ep, buf, frame_len, NULL, 0);
    if (err == ESP_OK) err = spi_poll(ep, pdMS_TO_TICKS(100), NULL); // attende fine tx
    if (err != ESP_OK) ESP_LOGE(TAG, "send frame fail ep=%d cmd=0x%02X err=%d", ep, command, err);

    heap_caps_free(buf);
    return err;
}

// --------- Parsing comune ---------
static esp_err_t parse_frame(spi_frame_t *f, size_t size)
{
    if (size < sizeof(frame_header_t)) 
        return ESP_ERR_INVALID_SIZE;

    if (f->header.start_byte != SPI_START_BYTE) 
        return ESP_ERR_INVALID_ARG;

    if (f->header.length > SPI_MAX_PAYLOAD_SIZE) 
        return ESP_ERR_INVALID_SIZE;

    size_t total = sizeof(frame_header_t) + f->header.length;

    if (total > size) 
        return ESP_ERR_INVALID_SIZE;

    uint16_t got = spi_crc16((const uint8_t*)&f->header, sizeof(frame_header_t) - sizeof(uint16_t) + f->header.length);
    
    if (got != f->header.checksum) 
        return ESP_ERR_INVALID_CRC;

    return ESP_OK;
}

static void handle_command(const spi_frame_t *in)
{
    switch (in->header.command) {
        case SPI_CMD_PING: {
            const char pong[] = "PONG";
            make_response(SPI_RSP_PONG, (const uint8_t*)pong, sizeof(pong));
            break;
        }
        case SPI_CMD_WIFI_ON: {
            make_response(SPI_RSP_WIFI_ON_ACK, NULL, 0);
            break;
        }
        case SPI_CMD_WIFI_SCAN: {
            const char demo[] = "ssid:demo,rssi:-42";
            make_response(SPI_RSP_WIFI_SCAN, (const uint8_t*)demo, sizeof(demo));
            break;
        }
        default: {
            break;
        }
    }
}

static void spi_rx_task(void *arg)
{
    (void)arg;
    spi_frame_t in;
    size_t len;

    while (1) {
        for (int ep = 0; ep < EP_MAX; ++ep) 
        {
            esp_err_t e = spi_try_recv_frame(ep, pdMS_TO_TICKS(10), &in, &len);
            if (e == ESP_OK) 
            {
                ESP_LOGI(TAG, "RX ep=%d cmd=0x%02X len=%u", ep, in.header.command, (unsigned)in.header.length);
                if (in.header.length) 
                {
                    char tmp[64];
                    size_t n = in.header.length < sizeof(tmp)-1 ? in.header.length : sizeof(tmp)-1;
                    memcpy(tmp, in.payload, n); tmp[n] = '\0';
                    ESP_LOGI(TAG, "Payload: %s", tmp);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

esp_err_t spi_protocol_init(void)
{
    xTaskCreate(spi_rx_task, "spi_rx_task", 4096, NULL, 5, &spi_rx_task_handle);
    return ESP_OK;
}