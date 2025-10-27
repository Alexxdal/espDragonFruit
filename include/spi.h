#ifndef SPI_H
#define SPI_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#if defined(BOARD_MASTER)
#include "driver/spi_master.h"
#else
#include "driver/spi_slave.h"
#endif

typedef enum {
    /**
     * @brief Slave 1 address (ESP-WROOM-32)
     */
    ESPWROOM32 = 0,
    /**
     * @brief Slave 2 address (ESP32C5)
     */
    ESP32C5,
    /**
     * @brief Slave 3 address (ESP32S3)
     */
    ESP32S3,
    SLAVE_NUM
} slave_addr_t;

esp_err_t spi_init(void);

esp_err_t spi_submit(int addr, uint8_t *tx_frame, uint8_t *rx_frame, TickType_t ticks);

#endif // SPI_H