#ifndef NETIF_H
#define NETIF_H

#include "esp_err.h"
#include "esp_netif.h"

/**
 * @brief Get the status of the network interface initialization.
 */
bool getNetIfStatus(void);


/**
 * @breif Initialize the network interface.
 */
esp_err_t network_interface_init(void);


/**
 * @brief Deinitialize the network interface.
 */
esp_err_t network_interface_deinit(void);


#endif // NETIF_H