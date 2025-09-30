#ifndef I2C_PROTO_H
#define I2C_PROTO_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "i2c.h"
#include "freertos/FreeRTOS.h"

typedef enum {
    CMD_PING = 0x01,
    CMD_PONG,
    CMD_WIFI_ON,
    CMD_WIFI_SCAN
} i2c_cmd_t;

#define I2C_MAX_PAYLOAD  128

typedef struct __attribute((packed)) {
    uint8_t cmd;
    uint16_t len;
    uint8_t crc;
} i2c_header_t;

typedef struct __attribute((packed)) {
    i2c_header_t header;
    uint8_t payload[I2C_MAX_PAYLOAD - sizeof(i2c_header_t)];
} i2c_frame_t;


esp_err_t i2c_send_frame(uint8_t addr, i2c_frame_t *frame, TickType_t to);

esp_err_t i2c_recv_frame(uint8_t addr, i2c_frame_t *frame, TickType_t to);

void i2c_slave_task(void *arg);

#endif // I2C_PROTO_H