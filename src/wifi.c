#include <string.h>
#include <esp_log.h>
#include "wifi.h"
#include "log.h"
#include "netif.h"

static const char *TAG = "WIFI";
static wifi_status_t wifi_status = 
{ 
    false,
    false
};


esp_err_t wifi_init_apsta(void)
{
    esp_err_t err = ESP_OK;

    if(wifi_status.wifi_initialized == true) {
        log_message(LOG_LEVEL_WARN, TAG, "WiFi already initialized");
        return ESP_OK;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize WiFi: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_start();
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to start WiFi: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_set_ps(WIFI_PS_NONE);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi power save mode: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi mode: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_set_max_tx_power(50);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi max TX power: %s", esp_err_to_name(err));
        return err;
    }

    const wifi_country_t country = {
        .cc = "CN",
        .schan = 1,
        .nchan = 14,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };
    err = esp_wifi_set_country(&country);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi country: %s", esp_err_to_name(err));
        return err;
    }
    wifi_status.wifi_initialized = true;

    log_message(LOG_LEVEL_INFO, TAG, "WiFi initialized in AP+STA mode");
    return ESP_OK;
}


esp_err_t wifi_start_softap(void)
{
    esp_err_t err = ESP_OK;

    if(wifi_status.wifi_initialized == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if(wifi_status.softap_running == true) {
        log_message(LOG_LEVEL_ERROR, TAG, "SoftAP already running");
        return ESP_OK;
    }

    if(getNetIfStatus() == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Network interface not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "espDragonFruit",
            .password = "espDragonFruit",
            .ssid_len = strlen("espDragonFruit"),
            .channel = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .beacon_interval = 300,
            .max_connection = 5,
            .pmf_cfg = {
                    /* Cannot set pmf to required when in wpa-wpa2 mixed mode! Setting pmf to optional mode. */
                    .required = false,
                    .capable = false
            }
        }
    };

    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set SoftAP config: %s", esp_err_to_name(err));
        return err;
    }
    wifi_status.softap_running = true;

    log_message(LOG_LEVEL_INFO, TAG, "SoftAP started with SSID:%s password:%s", wifi_config.ap.ssid, wifi_config.ap.password);
    return ESP_OK;
}
