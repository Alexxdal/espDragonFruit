#ifndef SPI_PROTO_H
#define SPI_PROTO_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "esp_wifi_types_generic.h"
#include "esp_chip_info.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "spi.h"
#include "board.h"

typedef enum {
    /* MASTER to SLAVE*/
    CMD_BOARD_STATUS = 0x01,
    CMD_WIFI_CONFIG,
    CMD_WIFI_CHANNEL,
    CMD_WIFI_SCAN,
    
    /* SLAVE to MASTER */
    CMD_BOARD_STATUS_RESPONSE,
    CMD_WIFI_CONFIG_RESPONSE,
    CMD_WIFI_CHANNEL_RESPONSE,
    CMD_WIFI_SCAN_RESULT,
} proto_cmd_t;

#define PROTO_MAX_PAYLOAD   (2048-sizeof(proto_header_t))
#define FRAME_QUEUE_SIZE    8


/**
 * @brief base frame header structures
 */
typedef struct __attribute((packed)) {
    uint8_t crc;
    uint8_t addr;
    uint16_t cmd;
    uint16_t len;
} proto_header_t;


/**
 * @brief base frame structure
 */
typedef struct __attribute((packed)) {
    proto_header_t header;
    uint8_t payload[PROTO_MAX_PAYLOAD];
} proto_frame_t;


/**
 * @brief Command to get Slave board status
 */
typedef struct __attribute((packed)) {
    proto_header_t header;
    struct __attribute__((packed)) {
        board_status_t status;
    } fields;
} proto_board_status_t;


/**
 * @brief Command to set Slave wifi
 */
typedef struct __attribute__((packed)) {
    proto_header_t header;
    struct __attribute__((packed)) {
        ap_config_t wifi_config_ap;
        sta_config_t wifi_config_sta;
        uint8_t wifi_mode;
        int32_t status;
    } fields;
} proto_wifi_config_t;


/**
 * @brief Packet to set wifi channel
 */
typedef struct __attribute((packed)) {
    proto_header_t header;
    struct __attribute((packed)) {
        uint8_t channel;
        int32_t status;
    } fields;
} proto_wifi_set_channel_t;


/**
 * @brief Initialize SPI bus and protocol Task
 */
esp_err_t spi_proto_init(void);


/**
 * @brief Add frame to queue
 * @param slave_addr Slave addres
 * @param frame Frame to send
 */
esp_err_t proto_send_frame(int slave_addr, const void *frame);


void proto_slave_task(void *arg);

void proto_master_task(void *arg);


#endif // SPI_PROTO_H