#include "netif.h"
#include "log.h"

static const char *TAG = "NETIF";
static bool netif_initialized = false;


bool getNetIfStatus(void)
{
    return netif_initialized;
}


esp_err_t network_interface_init(void)
{
    esp_err_t err = ESP_OK;

    if(netif_initialized == false)
    {
        err = esp_netif_init();
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize netif: %s", esp_err_to_name(err));
            return err;
        }
    }
    
    netif_initialized = true;
    log_message(LOG_LEVEL_INFO, TAG, "Network interface initialized.");
    return err;
}


esp_err_t netif_deinit(void)
{
    return esp_netif_deinit();
}