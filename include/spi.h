#ifndef SPI_H
#define SPI_H

#ifndef DMA_MAX
#define DMA_MAX 512
#endif

#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

/**
 * @brief Initialize the SPI bus.
 */
esp_err_t spi_init(void);

/**
 * @brief Submit an SPI transaction to a specified slave device.
 * 
 * @param slave_addr The address/index of the slave device (0, 1, or 2).
 * @param tx Pointer to the data to be transmitted.
 * @param tx_len Length of the data to be transmitted.
 * @param rx Pointer to the buffer to receive data.
 * @param rx_len Length of the buffer to receive data.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t spi_submit(int slave_addr, void *tx, size_t tx_len, void *rx, size_t rx_len);

/**
 * @brief Poll for the completion of an SPI transaction.
 * 
 * @param slave_addr The address/index of the slave device (0, 1, or 2).
 * @param to Timeout in ticks to wait for the transaction to complete.
 * @param rx_got Pointer to store the number of bytes received.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t spi_poll(int slave_addr, TickType_t to, size_t *rx_got);

#endif // SPI_H