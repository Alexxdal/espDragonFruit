#include <string.h>
#include <nvs_flash.h>
#include "log.h"
#include "nvs_config.h"

static const char *TAG = "NVS_CONFIG";


esp_err_t nvs_init(void)
{
    esp_err_t err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
        return err;
    }

    log_message(LOG_LEVEL_INFO, TAG, "NVS initialized successfully");
    return ESP_OK;
}


esp_err_t nvs_save_string(const char* key, const char* value)
{
    if(key == NULL || value == NULL) {
        log_message(LOG_LEVEL_ERROR, TAG, "Key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_str(nvs_handle, key, value);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set string in NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to commit changes to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    log_message(LOG_LEVEL_INFO, TAG, "String saved to NVS successfully: key=%s", key);
    return ESP_OK;
}


esp_err_t nvs_get_string(const char* key, char* value, size_t* length)
{
    if(key == NULL || value == NULL || length == NULL) {
        log_message(LOG_LEVEL_ERROR, TAG, "Key, value, or length is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_get_str(nvs_handle, key, value, length);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to get string from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    log_message(LOG_LEVEL_INFO, TAG, "String retrieved from NVS successfully: key=%s", key);
    return ESP_OK;
}