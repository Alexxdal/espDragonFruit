#ifndef BOARD_H
#define BOARD_H

#pragma once
#include "esp_err.h"

typedef enum {
    WIFI_IDLE = 0,
    WIFI_SCANNING,
    WIFI_AP_MODE,
    WIFI_STA_MODE,
    WIFI_APSTA_MODE,
    WIFI_CONNECTED,
    WIFI_DEAUTHING
} wifi_action_t;

typedef struct __attribute((packed)) {
    /* Module initialization status */
    uint8_t spi_status;
    uint8_t netif_status;
    uint8_t wifi_status;
    uint8_t bluetooth_status;
    
    /* WIFI */
    char ap_ssid[64];
    char ap_password[32];
    uint8_t ap_channel;
    wifi_action_t wifi_action;
    
} board_status_t;

/**
 * @brief Initialize the board based on its type (master or slave).
 */
esp_err_t board_init();

/**
 * @brief Get the current board status and settings
 */
board_status_t * getBoardStatus(void);

#endif // BOARD_H