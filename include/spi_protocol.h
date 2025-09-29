#ifndef SPI_PROTOCOL_H
#define SPI_PROTOCOL_H


#include <esp_system.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"


#define SPI_START_BYTE 0xA5
#define SPI_MAX_PAYLOAD_SIZE 256


#if defined(BOARD_MASTER)
#define EP_MAX 3
#else
#define EP_MAX 1
#endif


typedef enum {
    SPI_CMD_PING = 0x01,
    SPI_CMD_WIFI_ON = 0x02,
    SPI_CMD_WIFI_SCAN = 0x03,
    SPI_RSP_PONG = SPI_CMD_PING | 0x80, // 0x81
    SPI_RSP_WIFI_ON_ACK = SPI_CMD_WIFI_ON | 0x80, // 0x82
    SPI_RSP_WIFI_SCAN = SPI_CMD_WIFI_SCAN | 0x80, // 0x83
} spi_cmd_t;


// Header del frame (packed)
typedef struct __attribute__((packed)) 
{
    uint8_t start_byte;
    uint8_t command;
    uint16_t length;
    uint16_t checksum;
} frame_header_t;


// Frame completo
typedef struct __attribute__((packed)) 
{
    frame_header_t header;
    uint8_t payload[SPI_MAX_PAYLOAD_SIZE];
} spi_frame_t;


uint16_t spi_crc16(const uint8_t *data, size_t length);

esp_err_t spi_protocol_init(void);

esp_err_t spi_send_frame(int ep, uint8_t command, const uint8_t *payload, uint16_t length);

esp_err_t spi_try_recv_frame(int ep, TickType_t to, spi_frame_t *out, size_t *rx_len);

#endif // SPI_PROTOCOL_H