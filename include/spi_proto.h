#ifndef SPI_PROTO_H
#define SPI_PROTO_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "spi.h"

typedef enum {
    CMD_PING = 0x01,
    CMD_PONG,
    CMD_WIFI_ON,
    CMD_WIFI_SCAN
} proto_cmd_t;

#define PROTO_MAX_PAYLOAD   128
#define FRAME_QUEUE_SIZE     16

typedef struct __attribute((packed)) {
    uint8_t crc;
    uint8_t addr;
    uint16_t cmd;
    uint16_t len;
} proto_header_t;

typedef struct __attribute((packed)) {
    proto_header_t header;
    uint8_t payload[PROTO_MAX_PAYLOAD - sizeof(proto_header_t)];
} proto_frame_t;

esp_err_t spi_proto_init(void);

esp_err_t proto_send_frame(int slave_addr, proto_frame_t *frame);

void proto_slave_task(void *arg);

void proto_master_task(void *arg);


#endif // SPI_PROTO_H