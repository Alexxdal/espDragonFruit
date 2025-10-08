#include "netif.h"
#include "log.h"
#include "board.h"

static const char *TAG = "NETIF";

esp_err_t network_interface_init(void)
{
    esp_err_t err = ESP_OK;
    board_status_t *board_status = getBoardStatus();

    if(board_status->netif_status == false) {
        err = esp_netif_init();
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize netif: %s", esp_err_to_name(err));
            return err;
        }
    }
    
    board_status->netif_status = true;
    log_message(LOG_LEVEL_INFO, TAG, "Network interface initialized.");
    return err;
}


esp_err_t network_interface_deinit(void)
{
    board_status_t *board_status = getBoardStatus();
    esp_err_t err = esp_netif_deinit();
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to deinitialize netif: %s", esp_err_to_name(err));
        return err;
    }

    board_status->netif_status = false;
    log_message(LOG_LEVEL_INFO, TAG, "Network interface deinitialized.");
    return ESP_OK;
}