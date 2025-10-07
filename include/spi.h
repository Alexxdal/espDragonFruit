#ifndef I2C_H
#define I2C_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#if defined(BOARD_MASTER)
#include "driver/spi_master.h"

typedef enum {
    ESPWROOM32 = 1,
    ESP32C5,
    ESP32S3
} slave_addr_t;
#else
#include "driver/spi_slave.h"
#endif

esp_err_t spi_init(void);

esp_err_t spi_submit(int addr, uint8_t *tx_frame, uint8_t *rx_frame, TickType_t ticks);

#endif // I2C_H