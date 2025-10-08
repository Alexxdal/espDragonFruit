#include "driver/gpio.h"
#include "esp_check.h"
#include "spi.h"
#include "spi_proto.h"
#include "log.h"

#if defined(BOARD_MASTER)
static const gpio_num_t slave_cs[] = { SPI_S1_CS, SPI_S2_CS, SPI_S3_CS };
static spi_device_handle_t slave[3];
static spi_transaction_t transaction;
#else
static spi_slave_transaction_t transaction;
#endif

static const char *TAG = "SPI";

esp_err_t spi_init(void)
{
#if defined(BOARD_MASTER)
    static const spi_bus_config_t bus = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_SCLK,
        .data_io_default_level = 0,
        .max_transfer_sz = 4096,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST_TARGET, &bus, SPI_DMA_CH_AUTO));

    for(uint8_t i = 0; i < 3; i++)
    {
        spi_device_interface_config_t device = {
            .mode = 1,
            .clock_source = SPI_CLK_SRC_DEFAULT,
            .duty_cycle_pos = 128,
            .cs_ena_pretrans = 16,
            .cs_ena_posttrans = 16,
            .clock_speed_hz =  10000000,
            .spics_io_num = slave_cs[i],
            .queue_size = 16
        };
        ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST_TARGET, &device, &slave[i]));
    }

    log_message(LOG_LEVEL_INFO, TAG, "Master SPI Bus initialized");
    return ESP_OK;
#else
    static const spi_bus_config_t bus = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_SCLK,
        .data_io_default_level = 0,
        .max_transfer_sz = 4096,
        .flags = SPICOMMON_BUSFLAG_SLAVE,
    };
    static const spi_slave_interface_config_t slv = {
        .spics_io_num = SPI_CS,
        .flags = 0,
        .queue_size = 16,
        .mode = 1,
        .post_setup_cb = NULL,
        .post_trans_cb = NULL,
    };
    ESP_ERROR_CHECK(spi_slave_initialize(SPI_HOST_TARGET, &bus, &slv, SPI_DMA_CH_AUTO));

    log_message(LOG_LEVEL_INFO, TAG, "Slave SPI Bus initialized");
    return ESP_OK;
#endif
}


esp_err_t spi_submit(int addr, uint8_t *tx_frame, uint8_t *rx_frame, TickType_t ticks)
{
    if(tx_frame == NULL || rx_frame == NULL) {
        log_message(LOG_LEVEL_DEBUG, TAG, "rx frame or tx frame are null");
        return ESP_ERR_INVALID_ARG;
    }
    transaction.rx_buffer = rx_frame;
    transaction.tx_buffer = tx_frame;
    transaction.length    = (sizeof(proto_frame_t) * 8);
#if defined(BOARD_MASTER)
    esp_err_t e = spi_device_transmit(slave[addr], &transaction);
    return e;
#else
    (void)addr;
    esp_err_t e = spi_slave_transmit(SPI_HOST_TARGET, &transaction, ticks);
    return e;
#endif
}
