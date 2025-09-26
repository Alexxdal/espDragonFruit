#ifndef SPI_PROTOCOL_H
#define SPI_PROTOCOL_H

#include <esp_system.h>

#define SPI_START_BYTE 0xAA
#define SPI_MAX_PAYLOAD_SIZE 512

typedef struct {
    uint8_t start_byte;   // Start byte (e.g., 0xAA)
    uint8_t command;      // Command identifier
    uint16_t length;      // Length of the payload
    uint16_t checksum;     // 16Bit checksum (CRC-16)
} frame_header_t;

typedef struct {
    frame_header_t header;
    uint8_t payload[SPI_MAX_PAYLOAD_SIZE];
} spi_frame_t;


/**
 * @brief Calculate a simple checksum (CRC-16) for the given data.
 * 
 * @param data Pointer to the data buffer.
 * @param length Length of the data buffer.
 * @return uint16_t Calculated CRC-16 checksum.
 */
uint16_t calculate_checksum(const uint8_t *data, size_t length);


#endif // SPI_PROTOCOL_H