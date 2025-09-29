#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "spi.h"
#include "log.h"
#include "spi_protocol.h"


#if defined(BOARD_MASTER)
#include "driver/spi_master.h"
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
#include "driver/spi_slave.h"
#endif

static const char *TAG = "SPI";

static const spi_bus_config_t s_bus = {
    .mosi_io_num = SPI_MOSI,
    .miso_io_num = SPI_MISO,
    .sclk_io_num = SPI_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4096
};

#if defined(BOARD_MASTER)
static spi_device_handle_t s_dev[3];
static const int s_cs[3] = { SPI_S1_CS, SPI_S2_CS, SPI_S3_CS };
#else
// nothing extra
#endif


esp_err_t spi_init(void)
{
    esp_err_t err = ESP_OK;

#if defined(BOARD_MASTER)
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI_HOST_TARGET, &s_bus, SPI_DMA_CH_AUTO), TAG, "bus init");
    for (int i = 0; i < 3; ++i) 
    {
        spi_device_interface_config_t dev = {
            .clock_speed_hz = 4 * 1000 * 1000,
            .mode = 0,
            .spics_io_num = s_cs[i],
            .queue_size = 4,
            .cs_ena_posttrans = 3, // margine su ultimo bit
        };
        err = spi_bus_add_device(SPI_HOST_TARGET, &dev, &s_dev[i]);
        if(err != ESP_OK)
        {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to Add Slave %d.", i);
        }
    }
    log_message(LOG_LEVEL_INFO, TAG, "Master SPI ready");
    return ESP_OK;
#else
    spi_slave_interface_config_t cfg = {
        .mode = 0,
        .spics_io_num = SPI_CS,
        .queue_size = 4,
        .flags = 0,
    };
    ESP_RETURN_ON_ERROR(spi_slave_initialize(SPI_HOST_TARGET, &s_bus, &cfg, SPI_DMA_CH_AUTO), TAG, "slave init");
    ESP_LOGI(TAG, "Slave SPI ready");
    return ESP_OK;
#endif
}

esp_err_t spi_submit(int ep, void *tx, size_t tx_len, void *rx, size_t rx_len)
{
#if defined(BOARD_MASTER)
    (void)ep;
    spi_transaction_t *t = calloc(1, sizeof(*t));
    if (!t) return ESP_ERR_NO_MEM;
    size_t n = (tx_len > rx_len ? tx_len : rx_len);
    t->length   = n * 8;
    t->rxlength = rx_len ? rx_len * 8 : 0;
    t->tx_buffer = tx_len ? tx : NULL;
    t->rx_buffer = rx_len ? rx : NULL;

    esp_err_t e = spi_device_transmit(s_dev[ep], t);
    if (e != ESP_OK)
    {
        free(t);
    }
    return e;
#else
    (void)ep;
    spi_slave_transaction_t *t = calloc(1, sizeof(*t));
    if (!t) return ESP_ERR_NO_MEM;
    size_t n = (tx_len > rx_len ? tx_len : rx_len);
    t->length    = n * 8;
    t->tx_buffer = tx_len ? tx : NULL;
    t->rx_buffer = rx_len ? rx : NULL;

    esp_err_t e = spi_slave_transmit(SPI_HOST_TARGET, t, pdMS_TO_TICKS(50));
    if (e != ESP_OK)
    {
        free(t);
    }
    return e;
#endif
}