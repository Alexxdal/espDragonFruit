#ifndef I2C_H
#define I2C_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#if defined(BOARD_MASTER)
#include "driver/i2c_master.h"
#else
#include "driver/i2c_slave.h"
#include "driver/i2c.h"
#endif

#ifndef I2C_HZ
#define I2C_HZ   10000
#endif

#ifndef I2C_PORT
#define I2C_PORT I2C_NUM_0
#endif

#define SLAVE_RX_BUFF_LEN 256
#define SLAVE_TX_BUFF_LEN 256

esp_err_t i2c_init(void);

#if defined(BOARD_MASTER)
esp_err_t i2c_write_to(uint8_t addr, const uint8_t *data, size_t len, TickType_t to_ticks);
esp_err_t i2c_read_from(uint8_t addr, uint8_t *data, size_t len, TickType_t to_ticks);
int i2c_hasSlaveData(void);
#else
i2c_slave_dev_handle_t * getBus(void);
void i2c_drdy_ready(void);
void i2c_drdy_clear(void);
#endif

#endif // I2C_H