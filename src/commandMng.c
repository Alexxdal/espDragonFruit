#include <string.h>
#include "esp_err.h"
#include "commandMng.h"
#include "log.h"
#include "board.h"
#include "wifi.h"

static const char *TAG = "COMMAND_MNG";

#if defined(BOARD_MASTER)
void handle_frame_master(proto_frame_t *frame)
{
    //log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d with len=%d from slave=%d", frame->header.cmd, frame->header.len, frame->header.addr);
    switch (frame->header.cmd) {
        case CMD_BOARD_STATUS_RESPONSE: {
            proto_board_status_t *board_status_pkt = (proto_board_status_t *)frame;
            board_status_t *slave = getSlaveStatus(frame->header.addr);
            memcpy(slave, &board_status_pkt->fields.status, sizeof(board_status_t));
            break;
        }
        case CMD_WIFI_CONFIG_RESPONSE: {
            proto_wifi_config_t *wifi_config_pkt = (proto_wifi_config_t *)frame;
            board_status_t *slave = getSlaveStatus(frame->header.addr);
            if(wifi_config_pkt->fields.status == ESP_OK)
            {
                memcpy(&slave->wifi_config_ap, &wifi_config_pkt->fields.wifi_config_ap, sizeof(ap_config_t));
                memcpy(&slave->wifi_config_sta, &wifi_config_pkt->fields.wifi_config_sta, sizeof(sta_config_t));
                log_message(LOG_LEVEL_DEBUG, TAG, "Wifi Started on slave %d", frame->header.addr);
            }
            break;
        }
        case CMD_WIFI_CHANNEL_RESPONSE: {
            proto_wifi_set_channel_t *wifi_set_ch = (proto_wifi_set_channel_t *)frame;
            board_status_t *slave = getSlaveStatus(frame->header.addr);
            if(wifi_set_ch->fields.status == ESP_OK)
            {
                slave->wifi_config_ap.channel = wifi_set_ch->fields.channel;
                slave->wifi_config_sta.channel = wifi_set_ch->fields.channel;
            }
            break;
        }
        case CMD_WIFI_SCAN_RESPONSE: {
            proto_wifi_scan_t *wifi_scan_resp = (proto_wifi_scan_t *)frame;
            board_status_t *slave = getSlaveStatus(frame->header.addr);
            if(wifi_scan_resp->fields.status == ESP_OK)
            {
                log_message(LOG_LEVEL_DEBUG, TAG, "Wifi Scan Started on slave %d", frame->header.addr);
            }
            break;
        }
        case CMD_WIFI_SCAN_RESULTS_RESPONSE: {
            proto_wifi_scan_results_t *wifi_scan_results_resp = (proto_wifi_scan_results_t *)frame;
            board_status_t *slave = getSlaveStatus(frame->header.addr);
            if(wifi_scan_results_resp->fields.status == ESP_OK)
            {
                log_message(LOG_LEVEL_DEBUG, TAG, "Wifi Scan Results received from slave %d, APs found: %d", frame->header.addr, wifi_scan_results_resp->fields.scan_results.ap_num);
            }
            break;
        }
        default: {
            break;
        }
    }
}
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
proto_frame_t *handle_frame_slave(proto_frame_t *frame)
{
    static proto_frame_t response = { 0 };
    memset(&response, 0, sizeof(proto_frame_t));

    //log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d with len=%d from master", frame->header.cmd, frame->header.len);
    switch (frame->header.cmd) {
        case CMD_BOARD_STATUS: {
            proto_board_status_t *board_status_pkt = (proto_board_status_t *)frame;
            board_status_t *board_status = getBoardStatus();
            board_status_pkt->header.cmd = CMD_BOARD_STATUS_RESPONSE;
            board_status_pkt->header.len = sizeof(board_status_pkt->fields);
            board_status_pkt->fields.status = *board_status;
            memcpy(&response, board_status_pkt, sizeof(proto_board_status_t));
            return &response;
        }
        case CMD_WIFI_CONFIG: {
            proto_wifi_config_t *wifi_config_frame = (proto_wifi_config_t *)frame;
            esp_err_t err = wifi_set_config(&wifi_config_frame->fields.wifi_config_ap, &wifi_config_frame->fields.wifi_config_sta, wifi_config_frame->fields.wifi_mode);
            wifi_config_frame->header.cmd  = CMD_WIFI_CONFIG_RESPONSE;
            wifi_config_frame->header.len = sizeof(wifi_config_frame->fields);
            wifi_config_frame->fields.status = err;
            memcpy(&response, wifi_config_frame, sizeof(proto_wifi_config_t));
            return &response;
        }
        case CMD_WIFI_CHANNEL: {
            proto_wifi_set_channel_t *wifi_set_ch = (proto_wifi_set_channel_t *)frame;
            esp_err_t err = wifi_set_channel(wifi_set_ch->fields.channel);
            wifi_set_ch->header.cmd = CMD_WIFI_CHANNEL_RESPONSE;
            wifi_set_ch->header.len = sizeof(wifi_set_ch->fields);
            wifi_set_ch->fields.status = err;
            memcpy(&response, wifi_set_ch, sizeof(proto_wifi_set_channel_t));
            return &response;
        }
        case CMD_WIFI_SCAN: {
            proto_wifi_scan_t *wifi_scan_frame = (proto_wifi_scan_t *)frame;
            esp_err_t err = wifi_scan(&wifi_scan_frame->fields.scan_config);
            if(err != ESP_OK) {
                log_message(LOG_LEVEL_ERROR, TAG, "Failed to start WiFi scan: %s", esp_err_to_name(err));
            }
            wifi_scan_frame->header.cmd = CMD_WIFI_SCAN_RESPONSE;
            wifi_scan_frame->header.len = sizeof(wifi_scan_frame->fields);
            wifi_scan_frame->fields.status = err;
            memcpy(&response, wifi_scan_frame, sizeof(proto_wifi_scan_t));
            return &response;
        }
        default: {
            break;
        }
    }
    return NULL;
}
#endif


esp_err_t CommandSetWifiConfig(int addr, ap_config_t *config_ap, sta_config_t *config_sta, uint8_t mode)
{
    esp_err_t err = ESP_OK;
    proto_wifi_config_t wifi_config_pkt = { 0 };

    if(mode == WIFI_MODE_NULL)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "Wifi mode is null.");
        return ESP_ERR_INVALID_ARG;
    }
    if(mode == WIFI_MODE_AP && config_ap == NULL)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "config_ap is null!!!");
        return ESP_ERR_INVALID_ARG;
    }
    if(mode == WIFI_MODE_STA && config_sta == NULL)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "config_sta is null!!!");
        return ESP_ERR_INVALID_ARG;
    }
    if(mode == WIFI_MODE_APSTA && (!config_ap || !config_sta))
    {
        log_message(LOG_LEVEL_ERROR, TAG, "config_sta or config_ap are null!!!");
        return ESP_ERR_INVALID_ARG;
    }

    if(mode == WIFI_MODE_AP)
    {
        memcpy(&wifi_config_pkt.fields.wifi_config_ap, config_ap, sizeof(ap_config_t));
    }
    else if(mode == WIFI_MODE_STA)
    {
        memcpy(&wifi_config_pkt.fields.wifi_config_sta, config_sta, sizeof(sta_config_t));
    }
    else if(mode == WIFI_MODE_APSTA)
    {
        memcpy(&wifi_config_pkt.fields.wifi_config_ap, config_ap, sizeof(ap_config_t));
        memcpy(&wifi_config_pkt.fields.wifi_config_sta, config_sta, sizeof(sta_config_t));
    }

    wifi_config_pkt.header.cmd = CMD_WIFI_CONFIG;
    wifi_config_pkt.header.len = sizeof(wifi_config_pkt.fields);
    wifi_config_pkt.fields.wifi_mode = mode;

    err = proto_send_frame(addr, &wifi_config_pkt);
    if(err != ESP_OK)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to send wifi config frame.");
        return ESP_ERR_INVALID_STATE;
    }

    return err;
}

esp_err_t CommandSetWifiChannel(int addr, uint8_t channel)
{
    esp_err_t err = ESP_OK;
    proto_wifi_set_channel_t wifi_set_ch_pkt = { 0 };

    if(channel > 14 || channel == 0)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Invalid channel: %d", channel);
        return ESP_ERR_INVALID_ARG;
    }

    wifi_set_ch_pkt.header.cmd = CMD_WIFI_CHANNEL;
    wifi_set_ch_pkt.header.len = sizeof(wifi_set_ch_pkt.fields);
    wifi_set_ch_pkt.fields.channel = channel;

    err = proto_send_frame(addr, &wifi_set_ch_pkt);
    if(err != ESP_OK)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to send wifi set channel frame.");
        return ESP_ERR_INVALID_STATE;
    }

    return err;
}

esp_err_t CommandWifiScan(int addr, scan_config_t *scan_config)
{
    esp_err_t err = ESP_OK;
    proto_wifi_scan_t wifi_scan_pkt = { 0 };

    wifi_scan_pkt.header.cmd = CMD_WIFI_SCAN;
    wifi_scan_pkt.header.len = sizeof(wifi_scan_pkt.fields);
    if(scan_config != NULL)
    {
        memcpy(&wifi_scan_pkt.fields.scan_config, scan_config, sizeof(scan_config_t));
    }

    err = proto_send_frame(addr, &wifi_scan_pkt);
    if(err != ESP_OK)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to send wifi scan frame.");
        return ESP_ERR_INVALID_STATE;
    }

    return err;
}

/* ONLY SLAVE CALL THIS */
esp_err_t CommandWifiScanResults(int addr, scan_results_t *in_results)
{
    esp_err_t err = ESP_OK;
    proto_wifi_scan_results_t wifi_scan_results = { 0 };

    wifi_scan_results.header.cmd = CMD_WIFI_SCAN_RESULTS_RESPONSE;
    wifi_scan_results.header.len = sizeof(wifi_scan_results.fields);
    memcpy(&wifi_scan_results.fields.scan_results, in_results, sizeof(scan_results_t));
    wifi_scan_results.fields.status = ESP_OK;

    err = proto_send_frame(addr, &wifi_scan_results);
    if(err != ESP_OK)
    {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to send wifi scan frame.");
        return ESP_ERR_INVALID_STATE;
    }

    return err;
}