#include <string.h>
#include <esp_log.h>
#include "board.h"
#include "wifi.h"
#include "log.h"
#include "netif.h"
#include "commandMng.h"

static const char *TAG = "WIFI";

static esp_netif_t *netif_ap = NULL;
static esp_netif_t *netif_sta = NULL;
static TaskHandle_t s_wifi_worker = NULL;

static void wifi_worker_task(void *arg) 
{
    (void)arg;
    board_status_t *board_status = getBoardStatus();
    while(1)
    {
        uint32_t scan_result_error = 0;
        xTaskNotifyWait(0, 0xFFFFFFFF, &scan_result_error, portMAX_DELAY);

        if(scan_result_error == 0) {
            set_board_status {
                board_status->wifi_scan_started = false;
                board_status->wifi_scan_done = true;
                board_status->wifi_scan_error = 0;
            };
            scan_results_t *results = getCurrentBoardWifiScanResults();
            if (wifi_scan_get_results(results) == ESP_OK) {
                #if defined(BOARD_MASTER)
                log_message(LOG_LEVEL_DEBUG, TAG, "Wifi Scan Results, APs found: %d", results->ap_num);
                #else
                CommandWifiScanResults(SLAVE_ADDR, results);
                #endif
            }
            else
            {
                set_board_status {
                    board_status->wifi_scan_error = 1;
                };
            }
        }
        else {
            set_board_status {
                board_status->wifi_scan_started = false;
                board_status->wifi_scan_done = true;
                board_status->wifi_scan_error = 1;
            };
        }
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    board_status_t *board_status = getBoardStatus();
    switch(event_id)
    {
        case WIFI_EVENT_SCAN_DONE:
            wifi_event_sta_scan_done_t *e = (wifi_event_sta_scan_done_t *)event_data;
            xTaskNotify(s_wifi_worker, e->status, eSetValueWithOverwrite);
            break;

        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            set_board_status_single(board_status->wifi_sta_started, true);
            break;

        case WIFI_EVENT_STA_STOP:
            break;

        case WIFI_EVENT_STA_CONNECTED: {
            wifi_event_sta_connected_t *e = (wifi_event_sta_connected_t *)event_data;
            (void)e;
            set_board_status_single(board_status->wifi_sta_connected, true);
            break;
        }
        case WIFI_EVENT_STA_DISCONNECTED: {
            wifi_event_sta_disconnected_t *e = (wifi_event_sta_disconnected_t *)event_data;
            (void)e;
            set_board_status_single(board_status->wifi_sta_connected, false);
            break;
        }
        case WIFI_EVENT_AP_START:
            break;
        
        case WIFI_EVENT_AP_STOP:
            break;

        case WIFI_EVENT_AP_STACONNECTED: {
            wifi_event_ap_staconnected_t *e = (wifi_event_ap_staconnected_t *)event_data;
            (void)e;
            break;
        }
        case WIFI_EVENT_AP_STADISCONNECTED: {
            wifi_event_ap_stadisconnected_t *e = (wifi_event_ap_stadisconnected_t *)event_data;
            (void)e;
            break;
        }
        case WIFI_EVENT_FTM_REPORT: {
            wifi_event_ftm_report_t *e = (wifi_event_ftm_report_t *)event_data;
            (void)e;
            break;
        }
        case WIFI_EVENT_AP_WRONG_PASSWORD: {
            wifi_event_ap_wrong_password_t *e = (wifi_event_ap_wrong_password_t *)event_data;
            (void)e;
            break;
        }
    }
}

wifi_config_t *wifi_convert_sta_config(sta_config_t *config_sta)
{
    static wifi_config_t converted = { 0 };
    if(config_sta != NULL) {
        memset(&converted, 0, sizeof(converted));
        memcpy(&converted.sta.ssid, &config_sta->ssid, sizeof(config_sta->ssid));
        memcpy(&converted.sta.password, &config_sta->password, sizeof(config_sta->password));
        converted.sta.scan_method = config_sta->scan_method;
        converted.sta.bssid_set = config_sta->bssid_set;
        memcpy(&converted.sta.bssid, &config_sta->bssid, sizeof(config_sta->bssid));
        converted.sta.channel = config_sta->channel;
        converted.sta.listen_interval = config_sta->listen_interval;
        converted.sta.sort_method = config_sta->sort_method;
        converted.sta.threshold.rssi = config_sta->scan_threshold_rssi;
        converted.sta.threshold.authmode = config_sta->scan_threshold_authmode;
        converted.sta.threshold.rssi_5g_adjustment = config_sta->scan_threshold_rssi_5g_adjustment;
        converted.sta.pmf_cfg.capable = config_sta->pmf_capable;
        converted.sta.pmf_cfg.required = config_sta->pmf_required;
        converted.sta.rm_enabled = config_sta->rm_enabled;
        converted.sta.btm_enabled = config_sta->btm_enabled;
        converted.sta.mbo_enabled = config_sta->mbo_enabled;
        converted.sta.ft_enabled = config_sta->ft_enabled;
        converted.sta.owe_enabled = config_sta->owe_enabled;
        converted.sta.transition_disable = config_sta->transition_disable;
        converted.sta.sae_pwe_h2e = config_sta->sae_pwe_h2e;
        converted.sta.sae_pk_mode = config_sta->sae_pk_mode;
        converted.sta.failure_retry_cnt = config_sta->failure_retry_cnt;
        converted.sta.he_dcm_set = config_sta->he_dcm_set;
        converted.sta.he_dcm_max_constellation_tx = config_sta->he_dcm_max_constellation_tx;
        converted.sta.he_dcm_max_constellation_rx = config_sta->he_dcm_max_constellation_rx;
        converted.sta.he_mcs9_enabled = config_sta->he_mcs9_enabled;
        converted.sta.he_su_beamformee_disabled =config_sta->he_su_beamformee_disabled;
        converted.sta.he_trig_su_bmforming_feedback_disabled = config_sta->he_trig_su_bmforming_feedback_disabled;
        converted.sta.he_trig_mu_bmforming_partial_feedback_disabled = config_sta->he_trig_mu_bmforming_partial_feedback_disabled;
        converted.sta.vht_su_beamformee_disabled = config_sta->vht_su_beamformee_disabled;
        converted.sta.vht_mu_beamformee_disabled = config_sta->vht_mu_beamformee_disabled;
        converted.sta.vht_mcs8_enabled = config_sta->vht_mcs8_enabled;
        memcpy(&converted.sta.sae_h2e_identifier, &config_sta->sae_h2e_identifier, sizeof(config_sta->sae_h2e_identifier));
        return &converted;
    } else {
        return NULL;
    }
}

wifi_config_t *wifi_convert_ap_config(ap_config_t *config_ap)
{
    static wifi_config_t converted = { 0 };
    if(config_ap != NULL) {
        memset(&converted, 0, sizeof(converted));
        memcpy(&converted.ap.ssid, &config_ap->ssid, sizeof(config_ap->ssid));
        converted.ap.ssid_len = strlen((char *)converted.ap.ssid);
        memcpy(&converted.ap.password, &config_ap->password, sizeof(config_ap->password));
        converted.ap.channel = config_ap->channel;
        converted.ap.authmode = config_ap->authmode;
        converted.ap.ssid_hidden = config_ap->ssid_hidden;
        converted.ap.max_connection = config_ap->max_connection;
        converted.ap.beacon_interval = config_ap->beacon_interval;
        converted.ap.csa_count = config_ap->csa_count;
        converted.ap.dtim_period = config_ap->dtim_period;
        converted.ap.pairwise_cipher = config_ap->pairwise_cipher;
        converted.ap.ftm_responder = config_ap->ftm_responder;
        converted.ap.pmf_cfg.capable = config_ap->pmf_capable;
        converted.ap.pmf_cfg.required = config_ap->pmf_required;
        converted.ap.sae_pwe_h2e = config_ap->sae_pwe_h2e;
        converted.ap.transition_disable = config_ap->transition_disable;
        converted.ap.sae_ext = config_ap->sae_ext;
        converted.ap.bss_max_idle_cfg.period = config_ap->bss_max_idle_period;
        converted.ap.bss_max_idle_cfg.protected_keep_alive = config_ap->bss_max_idle_protected_keep_alive;
        converted.ap.gtk_rekey_interval = config_ap->gtk_rekey_interval;
        return &converted;
    } else {
        return NULL;
    }
}

esp_err_t wifi_set_config(ap_config_t *config_ap, sta_config_t *config_sta, uint8_t mode)
{
    esp_err_t err = ESP_OK;

    board_status_t *board_status = getBoardStatus();
    if(board_status == NULL) {
        log_message(LOG_LEVEL_DEBUG, TAG, "Failed to get board status");
        return ESP_ERR_INVALID_STATE;
    }

    if(mode == WIFI_MODE_NULL)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "Wifi mode is null.");
        return ESP_ERR_INVALID_ARG;
    }
    if(mode == WIFI_MODE_AP && !config_ap)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "AP config is null.");
        return ESP_ERR_INVALID_ARG;
    }
    if(mode == WIFI_MODE_STA && !config_sta)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "STA config is null.");
        return ESP_ERR_INVALID_ARG;
    }
    if(mode == WIFI_MODE_APSTA && (!config_ap || !config_sta))
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "AP or STA config are null.");
        return ESP_ERR_INVALID_ARG;
    }

    /* Start netif interfaces (create both once) */
    if(netif_ap == NULL)
    {
        netif_ap = esp_netif_create_default_wifi_ap();
        if(!netif_ap)
        {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to create default WiFi AP netif");
            return ESP_FAIL;
        }
    }
    if(netif_sta == NULL)
    {
        netif_sta = esp_netif_create_default_wifi_sta();
        if(!netif_sta)
        {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to create default WiFi AP netif");
            return ESP_FAIL;
        }
    }

    /* Init wifi */
    if(board_status->wifi_init == false)
    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        err = esp_wifi_init(&cfg);
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to initialize WiFi: %s", esp_err_to_name(err));
            return err;
        }

        err = esp_wifi_set_country_code("CN", true);
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi country: %s", esp_err_to_name(err));
            return err;
        }

        /* Start Scan Result Worker */
        if(s_wifi_worker == NULL) {
            xTaskCreate(wifi_worker_task, "wifi_worker", 8192, NULL, 5, &s_wifi_worker);
            if(s_wifi_worker == NULL) {
                log_message(LOG_LEVEL_ERROR, TAG, "Failed to create WiFi worker task");
                return ESP_ERR_NO_MEM;
            }
        }

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
        set_board_status_single(board_status->wifi_init, true);
    }

    /* Stop wifi before changing settings (if already started) */
    if(board_status->wifi_started == true)
    {
        esp_wifi_stop();
        set_board_status_single(board_status->wifi_started, false);
    }

    /* Set wifi mode */
    err = esp_wifi_set_mode(mode);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi mode: %s", esp_err_to_name(err));
        return err;
    }

    /* Power save only relevant for STA/APSTA (moved here) */
    if(mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
    {
        err = esp_wifi_set_ps(WIFI_PS_NONE);
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi power save mode: %s", esp_err_to_name(err));
            return err;
        }
    }

    /* Set config */
    if(mode == WIFI_MODE_AP)
    {
        err = esp_wifi_set_config(WIFI_IF_AP, wifi_convert_ap_config(config_ap));
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to set SoftAP config: %s", esp_err_to_name(err));
            return err;
        }

        set_board_status {
            memcpy(&board_status->wifi_config_ap, config_ap, sizeof(ap_config_t));
        }
    }
    else if(mode == WIFI_MODE_STA)
    {
        err = esp_wifi_set_config(WIFI_IF_STA, wifi_convert_sta_config(config_sta));
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to set STA config: %s", esp_err_to_name(err));
            return err;
        }
        set_board_status {
            memcpy(&board_status->wifi_config_sta, config_sta, sizeof(sta_config_t));
        }
    }
    else if(mode == WIFI_MODE_APSTA)
    {
        err = esp_wifi_set_config(WIFI_IF_AP, wifi_convert_ap_config(config_ap));
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to set SoftAP config: %s", esp_err_to_name(err));
            return err;
        }
        err = esp_wifi_set_config(WIFI_IF_STA, wifi_convert_sta_config(config_sta));
        if(err != ESP_OK) {
            log_message(LOG_LEVEL_ERROR, TAG, "Failed to set STA config: %s", esp_err_to_name(err));
            return err;
        }
        set_board_status {
            memcpy(&board_status->wifi_config_ap, config_ap, sizeof(ap_config_t));
            memcpy(&board_status->wifi_config_sta, config_sta, sizeof(sta_config_t));
        }
    }
    
    /* Start wifi */
    err = esp_wifi_start();
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to start WiFi: %s", esp_err_to_name(err));
        return err;
    }

    /* Set TX Max power after wifi is started */
    err = esp_wifi_set_max_tx_power(50);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to set WiFi max TX power: %s", esp_err_to_name(err));
        return err;
    }

    set_board_status_single(board_status->wifi_started, true);
    set_board_status_single(board_status->wifi_mode, mode);

    log_message(LOG_LEVEL_INFO, TAG, "Wifi initialized and started.");
    return err;
}

esp_err_t wifi_set_channel(uint8_t channel)
{
    board_status_t *status = getBoardStatus();
    if(channel > 14 || channel == 0)
        return ESP_ERR_INVALID_ARG;
    
    if(status->wifi_init == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant change channel, WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_started == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant change channel, WiFi not started");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    if(err == ESP_OK) {
        set_board_status {
            status->wifi_config_ap.channel = channel;
            status->wifi_config_sta.channel = channel;
        }
    }
    return err;
}

esp_err_t wifi_scan(scan_config_t *scan_config)
{
    board_status_t *status = getBoardStatus();
    esp_err_t err = ESP_OK;

    if(status->wifi_init == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant Scan, WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_started == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant scan, WiFi not started");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_sta_started == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant scan, WiFi STA not started (need STA or APSTA mode)");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_scan_started == true) {
        log_message(LOG_LEVEL_WARN, TAG, "WiFi scan already in progress");
        return ESP_ERR_INVALID_STATE;
    }
    /* If scan_config is NULL default value will be used */
    wifi_scan_config_t esp_scan_config = { 0 };
    if(scan_config != NULL) {
        esp_scan_config.channel = scan_config->channel;
        esp_scan_config.show_hidden = scan_config->show_hidden;
        esp_scan_config.scan_type = scan_config->scan_type;
        esp_scan_config.scan_time.active.min = scan_config->scan_time;
        esp_scan_config.scan_time.active.max = scan_config->scan_time;
        esp_scan_config.channel_bitmap.ghz_2_channels = scan_config->ghz_2_channel_bitmap;
        esp_scan_config.channel_bitmap.ghz_5_channels = scan_config->ghz_5_channel_bitmap;
        err = esp_wifi_scan_start(&esp_scan_config, false);
    } else {
        err = esp_wifi_scan_start(NULL, false);
    }
    if(err == ESP_OK) {
        set_board_status {
            status->wifi_scan_started = true;
            status->wifi_scan_done = false;
            status->wifi_scan_error = 0;
        }
        log_message(LOG_LEVEL_DEBUG, TAG, "Wifi Scan Started.");
    } else {
        set_board_status {
            status->wifi_scan_started = false;
            status->wifi_scan_done = false;
            status->wifi_scan_error = 1;
        }
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to start WiFi scan: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t wifi_scan_get_results(scan_results_t *out_results)
{
    board_status_t *status = getBoardStatus();
    if(status->wifi_init == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant get scan results, WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_started == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant get scan results, WiFi not started");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_sta_started == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant get scan results, WiFi STA not started");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_scan_done == false) {
        log_message(LOG_LEVEL_ERROR, TAG, "Cant get scan results, WiFi scan not done");
        return ESP_ERR_INVALID_STATE;
    }
    if(status->wifi_scan_error == 1) {
        log_message(LOG_LEVEL_WARN, TAG, "WiFi scan ended with error");
        return ESP_ERR_NOT_FOUND;
    }

    uint16_t ap_num = 0;
    esp_err_t err = esp_wifi_scan_get_ap_num(&ap_num);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to get number of scanned APs: %s", esp_err_to_name(err));
        return err;
    }
    if(ap_num > WIFI_SCAN_MAX_AP) ap_num = WIFI_SCAN_MAX_AP; // Limit to max we can store

    wifi_ap_record_t ap_records[WIFI_SCAN_MAX_AP];
    err = esp_wifi_scan_get_ap_records(&ap_num, ap_records);
    if(err != ESP_OK) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to get scanned AP records: %s", esp_err_to_name(err));
        return err;
    }

    out_results->ap_num = ap_num;
    for(int i = 0; i < ap_num; i++) {
        memcpy(out_results->results[i].bssid, ap_records[i].bssid, 6);
        memcpy(out_results->results[i].ssid, ap_records[i].ssid, 33);
        out_results->results[i].primary_channel = ap_records[i].primary;
        out_results->results[i].secondary_channel = ap_records[i].second;
        out_results->results[i].rssi = ap_records[i].rssi;
        out_results->results[i].authmode = ap_records[i].authmode;
        out_results->results[i].pairwise_cipher = ap_records[i].pairwise_cipher;
        out_results->results[i].group_cipher = ap_records[i].group_cipher;
        out_results->results[i].ant = ap_records[i].ant;
        out_results->results[i].phy_11b = ap_records[i].phy_11b;
        out_results->results[i].phy_11g = ap_records[i].phy_11g;
        out_results->results[i].phy_11n = ap_records[i].phy_11n;
        out_results->results[i].phy_lr = ap_records[i].phy_lr;
        out_results->results[i].phy_11a = ap_records[i].phy_11a;
        out_results->results[i].phy_11ac = ap_records[i].phy_11ac;
        out_results->results[i].phy_11ax = ap_records[i].phy_11ax;
        out_results->results[i].wps = ap_records[i].wps;
        out_results->results[i].ftm_responder = ap_records[i].ftm_responder;
        out_results->results[i].ftm_initiator = ap_records[i].ftm_initiator;
        memcpy(out_results->results[i].country_code, ap_records[i].country.cc, 3);
        out_results->results[i].schan = ap_records[i].country.schan;
        out_results->results[i].nchan = ap_records[i].country.nchan;
        out_results->results[i].max_tx_power = ap_records[i].country.max_tx_power;
        out_results->results[i].policy = ap_records[i].country.policy;
        #if CONFIG_SOC_WIFI_SUPPORT_5G
        out_results->results[i].wifi_5g_channel_mask = ap_records[i].country.wifi_5g_channel_mask;
        #endif
        out_results->results[i].bss_color = ap_records[i].he_ap.bss_color;
        out_results->results[i].partial_bss_color = ap_records[i].he_ap.partial_bss_color;
        out_results->results[i].bss_color_disabled = ap_records[i].he_ap.bss_color_disabled;
        out_results->results[i].bssid_index = ap_records[i].he_ap.bssid_index;
        out_results->results[i].bandwidth = ap_records[i].bandwidth;
        out_results->results[i].vht_ch_freq1 = ap_records[i].vht_ch_freq1;
        out_results->results[i].vht_ch_freq2 = ap_records[i].vht_ch_freq2;
    }

    return err;
}

