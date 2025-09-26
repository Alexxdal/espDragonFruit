#include "spi.h"
#include "spi_protocol.h"


DMA_ATTR static uint8_t tx[DMA_MAX];
DMA_ATTR static uint8_t rx[DMA_MAX];

uint16_t calculate_checksum(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}