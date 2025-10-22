#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
#include "esp_wifi.h"

#define WIFI_SCAN_MAX_AP 20

/**
 * @brief Access Point configuration structure
 */
typedef struct __attribute((packed)) {
    uint8_t ssid[32];
    uint8_t password[64];
    uint8_t channel;
    uint8_t authmode;
    uint8_t ssid_hidden;
    uint8_t max_connection;
    uint16_t beacon_interval;
    uint8_t csa_count;
    uint8_t dtim_period;
    uint8_t pairwise_cipher;
    uint8_t ftm_responder;
    uint8_t pmf_capable;
    uint8_t pmf_required;
    uint8_t sae_pwe_h2e;
    uint8_t transition_disable;
    uint8_t sae_ext;
    uint16_t bss_max_idle_period;
    uint8_t bss_max_idle_protected_keep_alive;
    uint16_t gtk_rekey_interval;
} ap_config_t;

/**
 * @brief Station configuration structure
 */
typedef struct __attribute((packed)) {
    uint8_t ssid[32];
    uint8_t password[64];
    uint8_t scan_method;
    uint8_t bssid_set; 
    uint8_t bssid[6];
    uint8_t channel;
    uint16_t listen_interval; 
    uint8_t sort_method;
    int8_t scan_threshold_rssi;
    uint8_t scan_threshold_authmode;
    uint8_t scan_threshold_rssi_5g_adjustment;
    uint8_t pmf_capable;
    uint8_t pmf_required;
    uint8_t rm_enabled;
    uint8_t btm_enabled;
    uint8_t mbo_enabled;
    uint8_t ft_enabled;
    uint8_t owe_enabled;
    uint8_t transition_disable;
    uint8_t sae_pwe_h2e;
    uint8_t sae_pk_mode;
    uint8_t failure_retry_cnt;
    uint8_t he_dcm_set;
    uint8_t he_dcm_max_constellation_tx;
    uint8_t he_dcm_max_constellation_rx;
    uint8_t he_mcs9_enabled;
    uint8_t he_su_beamformee_disabled;
    uint8_t he_trig_su_bmforming_feedback_disabled;
    uint8_t he_trig_mu_bmforming_partial_feedback_disabled;
    uint8_t vht_su_beamformee_disabled;
    uint8_t vht_mu_beamformee_disabled;
    uint8_t vht_mcs8_enabled;
    uint8_t sae_h2e_identifier[32];
} sta_config_t;

/**
 * @brief Scan result structure
 */
typedef struct __attribute((packed)) {
    uint8_t bssid[6];
    uint8_t ssid[33];
    uint8_t primary_channel;
    uint8_t secondary_channel;
    int8_t rssi;
    uint8_t authmode;
    uint8_t pairwise_cipher;
    uint8_t group_cipher;
    uint8_t ant;
    uint8_t phy_11b;
    uint8_t phy_11g;
    uint8_t phy_11n;
    uint8_t phy_lr;
    uint8_t phy_11a;
    uint8_t phy_11ac;
    uint8_t phy_11ax;
    uint8_t wps;
    uint8_t ftm_responder;
    uint8_t ftm_initiator;
    /* wifi_country_t */
    char country_code[3];
    uint8_t schan;
    uint8_t nchan;
    uint8_t max_tx_power;
    uint8_t policy;
    uint32_t wifi_5g_channel_mask;
    /* wifi_he_ap_info_t */
    uint8_t bss_color;
    uint8_t partial_bss_color;
    uint8_t bss_color_disabled;
    uint8_t bssid_index;
    /* --------------- */
    uint8_t bandwidth;
    uint8_t vht_ch_freq1;
    uint8_t vht_ch_freq2;
} scan_result_t;

typedef struct __attribute((packed)) {
    uint8_t channel;
    uint8_t show_hidden;
    uint8_t scan_type;
    uint8_t scan_time;
    uint16_t ghz_2_channel_bitmap;
    uint32_t ghz_5_channel_bitmap;
    uint16_t alignment_bytes; // Padding for alignment
} scan_config_t;

/**
 * @brief Wifi scan result structure
 */
typedef struct __attribute((packed)) {
    uint8_t ap_num;
    scan_result_t results[WIFI_SCAN_MAX_AP];
} scan_results_t;

/**
 * @brief Set wifi config and start
 * @param config_ap Wifi AP config
 * @param config_sta Wifi STA config
 * @param mode Wifi mode to set (AP, STA, APSTA)
 */
esp_err_t wifi_set_config(ap_config_t *config_ap, sta_config_t *config_sta, uint8_t mode);

/**
 * @brief Set Wifi Channel
 * @param channel channel to set (1 - 14)
 */
esp_err_t wifi_set_channel(uint8_t channel);

/**
 * @brief Start Wifi Scan
 * @param scan_config Wifi scan configuration (NULL for default)
 */
esp_err_t wifi_scan(scan_config_t *scan_config);

/**
 * @brief Get Wifi Scan Results
 * @param out_results Pointer to store scan results - scan_results_t
 */
esp_err_t wifi_scan_get_results(scan_results_t *out_results);


#endif // WIFI_H