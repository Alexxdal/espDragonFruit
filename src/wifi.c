#include "wifi.h"

static const char *TAG = "WIFI";


esp_err_t wifi_init_apsta(void)
{
    esp_err_t ret = ESP_OK;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_start();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_set_ps(WIFI_PS_NONE);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi power save mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
        return ret;
    }

    const wifi_country_t country = {
        .cc = "CN",
        .schan = 1,
        .nchan = 14,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };
    ret = esp_wifi_set_country(&country);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi country: %s", esp_err_to_name(ret));
        return ret;
    }


    ESP_LOGI(TAG, "WiFi initialized in AP+STA mode");
    return ESP_OK;
}
