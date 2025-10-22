#ifndef BOARD_H
#define BOARD_H

#pragma once
#include "esp_wifi_types_generic.h"
#include "esp_chip_info.h"
#include "esp_err.h"
#include "wifi.h"

#define set_board_status for (int _once=(board_status_lock(),0); !_once; _once=(board_status_unlock(),1))
#define set_board_status_single(field, value) board_status_lock(); field = value; board_status_unlock();

/**
 * @brief Structure to hold the current board status and settings
 */
typedef struct __attribute((packed)) {
    /* Board info */
    uint8_t  chip_cores;
    uint8_t  chip_model;
    uint16_t  chip_revision;
    uint32_t chip_features;

    /* RAM */
    uint32_t total_internal_memory;
    uint32_t free_internal_memory;
    uint32_t largest_contig_internal_block;
    size_t spiram_size;

    /* Module initialization status */
    uint8_t spi_status;
    uint8_t netif_status;
    uint8_t wifi_init;
    uint8_t wifi_started;
    uint8_t bluetooth_status;
    
    /* WIFI */
    ap_config_t wifi_config_ap;
    sta_config_t wifi_config_sta;
    uint8_t wifi_mode;
    uint8_t wifi_ready;
    /* Scan */
    uint8_t wifi_scan_started;
    uint8_t wifi_scan_done;
    uint8_t wifi_scan_ap_num;
    uint8_t wifi_scan_error;

    uint8_t wifi_sta_started;
    uint8_t wifi_sta_connected;
    uint8_t wifi_ap_started;
    uint8_t wifi_ap_has_clients;

} board_status_t;

/**
 * @brief Initialize the board based on its type (master or slave).
 */
esp_err_t board_init();

/**
 * @brief Get the current board status
 */
board_status_t *getBoardStatus(void);

/**
 * @brief Get the status of a slave board (master only)
 * @param addr Slave address (1-based index)
 */
board_status_t *getSlaveStatus(int addr);

/**
 * @brief Lock board status mutex
 */
void board_status_lock(void);

/**
 * @brief Unlock board status mutex
 */
void board_status_unlock(void);

#endif // BOARD_H