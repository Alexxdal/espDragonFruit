#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_check.h"
#include "spi.h"
#include "log.h"

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
  .max_transfer_sz = 4069
};

static TaskHandle_t spi_pool_task_handle;

#if defined(BOARD_MASTER)
static spi_device_handle_t s_dev[3];
static const int s_cs[3] = { SPI_S1_CS, SPI_S2_CS, SPI_S3_CS };
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
static spi_slave_transaction_t s_t;
static bool s_ready = false; 
#endif


static void spi_pool_task(void *pvParameters)
{
    (void)pvParameters;
    esp_err_t err = ESP_OK;

    #if defined(BOARD_MASTER)
    size_t rx_got = 0;
    while (1) 
    {
        for (int i=0; i<3; i++) 
        {
            err = spi_poll(i, portMAX_DELAY, &rx_got);
            if (err == ESP_OK && rx_got > 0) 
            {
                log_message(LOG_LEVEL_INFO, TAG, "Received %d bytes from slave %d", rx_got, i);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    #elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
    size_t rx_got = 0;
    while (1) 
    {
        err = spi_poll(0, portMAX_DELAY, &rx_got);
        if (err == ESP_OK && rx_got > 0) 
        {
            log_message(LOG_LEVEL_INFO, TAG, "Received %d bytes from master", rx_got);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    #endif
    vTaskDelete(NULL);
}


esp_err_t spi_init(void)
{
    #if defined(BOARD_MASTER)
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI_HOST_TARGET, &s_bus, SPI_DMA_CH_AUTO), TAG, "bus init");
    log_message(LOG_LEVEL_DEBUG, TAG, "SPI bus initialized.");
    for (int i=0; i<3; i++) 
    {
        spi_device_interface_config_t dev = {
            .clock_speed_hz = 10000000, //10MHZ
            .mode = 0,
            .spics_io_num = s_cs[i],
            .queue_size = 4,
        };
        ESP_RETURN_ON_ERROR(spi_bus_add_device(SPI_HOST_TARGET, &dev, &s_dev[i]), TAG, "add dev");
        log_message(LOG_LEVEL_DEBUG, TAG, "Added SPI slave: %d", i);
    }
    #elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
    spi_slave_interface_config_t cfg = 
    { 
        .mode=0,
        .spics_io_num = SPI_CS,
        .queue_size = 4 
    };
    ESP_RETURN_ON_ERROR(spi_slave_initialize(SPI_HOST_TARGET, &s_bus, &cfg, SPI_DMA_CH_AUTO), TAG, "slave");
    memset(&s_t, 0, sizeof(s_t));
    #endif

    /* Start SPI Pool Task */
    xTaskCreate(spi_pool_task, "spi_pool_task", 2048, NULL, 5, &spi_pool_task_handle);

    log_message(LOG_LEVEL_INFO, TAG, "SPI Initialized.");
    return ESP_OK;
}


esp_err_t spi_submit(int slave_addr, void *tx, size_t tx_len, void *rx, size_t rx_len)
{
    #if defined(BOARD_MASTER)
    spi_transaction_t *t = calloc(1, sizeof(*t));
    if (!t) return ESP_ERR_NO_MEM;
    t->length    = 8 * (tx_len > rx_len ? tx_len : rx_len);
    t->tx_buffer = tx_len ? tx : NULL;
    t->rx_buffer = rx_len ? rx : NULL;
    esp_err_t e = spi_device_queue_trans(s_dev[slave_addr], t, 0);
    if (e != ESP_OK) free(t);
    return e;

    #elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
    (void)slave_addr;
    if (s_ready) return ESP_ERR_INVALID_STATE;
    memset(&s_t, 0, sizeof(s_t));
    s_t.length    = 8 * (tx_len > rx_len ? tx_len : rx_len);
    s_t.tx_buffer = tx_len ? tx : NULL;
    s_t.rx_buffer = rx_len ? rx : NULL;
    s_ready = true;
    return ESP_OK;
    #endif
}


esp_err_t spi_poll(int slave_addr, TickType_t to, size_t *rx_got)
{
    #if defined(BOARD_MASTER)
    spi_transaction_t *ret = NULL;
    esp_err_t e = spi_device_get_trans_result(s_dev[slave_addr], &ret, to);
    if (e != ESP_OK) return e;
    if (rx_got) *rx_got = ret->rx_buffer ? (ret->length/8) : 0;
    free(ret); // la struct t l’abbiamo allocata noi; i buffer li gestisce il chiamante
    return ESP_OK;

    #elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
    (void)slave_addr; // non usato
    if (!s_ready) return ESP_ERR_INVALID_STATE;  // niente da scambiare
    esp_err_t e = spi_slave_transmit(SPI_HOST_TARGET, &s_t, to); // blocca max 'to'
    if (e != ESP_OK) return e; // timeout → nessun clock dal master
    if (rx_got) *rx_got = s_t.rx_buffer ? (s_t.length/8) : 0;
    s_ready = false; // consumata; per il prossimo giro servirà un nuovo submit()
    return ESP_OK;
    #endif
}
