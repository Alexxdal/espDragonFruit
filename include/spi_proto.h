#ifndef SPI_PROTO_H
#define SPI_PROTO_H

#include <stdint.h>
#include <stddef.h>
#include "esp_chip_info.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "spi.h"

typedef enum {
    /* MASTER */
    CMD_POLL = 0x01,
    CMD_CHIP_INFO,
    CMD_WIFI_SCAN,

    /* SLAVE */
    CMD_ACK,
    CMD_CHIP_INFO_RESPONSE
} proto_cmd_t;

#define PROTO_MAX_PAYLOAD   256
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

typedef struct __attribute((packed)) {
    proto_header_t header;
    struct {
        esp_chip_info_t chip;
    } fields;
} proto_chip_info_t;

esp_err_t spi_proto_init(void);

esp_err_t proto_send_frame(int slave_addr, void *frame);

void proto_slave_task(void *arg);

void proto_master_task(void *arg);


#endif // SPI_PROTO_H