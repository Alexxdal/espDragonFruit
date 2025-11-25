#include "utility.h"
#include <string.h>

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

const char *wifi_auth_mode_to_str(uint8_t authmode)
{
    switch ((wifi_auth_mode_t)authmode) {
        case WIFI_AUTH_OPEN:                 return "OPEN";
        case WIFI_AUTH_WEP:                  return "WEP";
        case WIFI_AUTH_WPA_PSK:              return "WPA-PSK";
        case WIFI_AUTH_WPA2_PSK:             return "WPA2-PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:         return "WPA/WPA2-PSK";
        case WIFI_AUTH_ENTERPRISE:           return "WPA2-Enterprise";
        case WIFI_AUTH_WPA3_PSK:             return "WPA3-PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK:        return "WPA2/WPA3-PSK";
        case WIFI_AUTH_WAPI_PSK:             return "WAPI-PSK";
        case WIFI_AUTH_OWE:                  return "OWE";
        case WIFI_AUTH_WPA3_ENT_192:         return "WPA3-Enterprise-192";
        case WIFI_AUTH_WPA3_EXT_PSK:         return "WPA3-PSK-EXT";
        case WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE:
                                             return "WPA3-PSK-MIXED";
        case WIFI_AUTH_DPP:                  return "DPP";
        case WIFI_AUTH_WPA3_ENTERPRISE:      return "WPA3-Enterprise";
        case WIFI_AUTH_WPA2_WPA3_ENTERPRISE: return "WPA2/WPA3-Enterprise";
        case WIFI_AUTH_WPA_ENTERPRISE:       return "WPA-Enterprise";
        default:                             return "UNKNOWN";
    }
}

const char *wifi_cipher_type_to_str(uint8_t cipher)
{
    switch ((wifi_cipher_type_t)cipher) {
        case WIFI_CIPHER_TYPE_NONE:          return "NONE";
        case WIFI_CIPHER_TYPE_WEP40:         return "WEP40";
        case WIFI_CIPHER_TYPE_WEP104:        return "WEP104";
        case WIFI_CIPHER_TYPE_TKIP:          return "TKIP";
        case WIFI_CIPHER_TYPE_CCMP:          return "CCMP";
        case WIFI_CIPHER_TYPE_TKIP_CCMP:     return "TKIP+CCMP";
        case WIFI_CIPHER_TYPE_AES_CMAC128:   return "AES-CMAC-128";
        case WIFI_CIPHER_TYPE_SMS4:          return "SMS4";
        case WIFI_CIPHER_TYPE_GCMP:          return "GCMP";
        case WIFI_CIPHER_TYPE_GCMP256:       return "GCMP-256";
        case WIFI_CIPHER_TYPE_AES_GMAC128:   return "AES-GMAC-128";
        case WIFI_CIPHER_TYPE_AES_GMAC256:   return "AES-GMAC-256";
        case WIFI_CIPHER_TYPE_UNKNOWN:
        default:                             return "UNKNOWN";
    }
}