#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
#include "esp_wifi.h"

/**
 * @brief Initialize WiFi in AP+STA mode
 */
esp_err_t wifi_init_apsta(void);

/**
 * @brief Start the WiFi SoftAP
 */
esp_err_t wifi_start_softap(void);

#endif // WIFI_H