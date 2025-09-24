#ifndef NETIF_H
#define NETIF_H

#include "esp_err.h"
#include "esp_netif.h"

bool getNetIfStatus(void);
esp_err_t network_interface_init(void);
esp_err_t network_interface_deinit(void);

#endif // NETIF_H