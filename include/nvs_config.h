#ifndef NVS_CONFIG_H
#define NVS_CONFIG_H

#include "esp_err.h"

/**
 * @brief Initialize Non-Volatile Storage (NVS).
 */
esp_err_t nvs_init(void);


/**
 * @brief Save a string value to NVS.
 */
esp_err_t nvs_save_string(const char* key, const char* value);


/**
 * @brief Retrieve a string value from NVS.
 */
esp_err_t nvs_get_string(const char* key, char* value, size_t* length);


#endif // NVS_CONFIG_H
