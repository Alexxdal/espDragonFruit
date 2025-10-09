#ifndef SPI_PROTO_H
#define SPI_PROTO_H

#include <stdint.h>
#include <stddef.h>
#include "esp_chip_info.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "spi.h"
#include "board.h"

typedef enum {
    /* MASTER */
    CMD_BOARD_STATUS = 0x01,
    CMD_MEM_INFO,

    /* SLAVE */
    CMD_ACK,
    CMD_BOARD_STATUS_RESPONSE,
    CMD_MEM_INFO_RESPONSE
} proto_cmd_t;

#define PROTO_MAX_PAYLOAD   2048
#define FRAME_QUEUE_SIZE    4

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
        board_status_t status;
    } fields;
} proto_board_status_t;

esp_err_t spi_proto_init(void);

esp_err_t proto_send_frame(int slave_addr, void *frame);

void proto_slave_task(void *arg);

void proto_master_task(void *arg);


#endif // SPI_PROTO_H