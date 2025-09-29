#ifndef I2C_PROTO_H
#define I2C_PROTO_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "i2c.h"
#include "freertos/FreeRTOS.h"

typedef enum {
  CMD_PING      = 0x01,
  CMD_WIFI_ON   = 0x02,
  CMD_WIFI_SCAN = 0x03,
} i2c_cmd_t;

#define I2C_MAX_PAYLOAD  128

esp_err_t i2c_send_cmd(uint8_t addr, uint8_t cmd, const uint8_t *payload, uint8_t len, TickType_t to);
esp_err_t i2c_recv_resp(uint8_t addr, uint8_t *buf, uint8_t bufsize, uint8_t *out_len, TickType_t to);

void i2c_slave_task(void *arg);

#endif // I2C_PROTO_H