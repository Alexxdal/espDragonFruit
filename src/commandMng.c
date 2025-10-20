#include <string.h>
#include "esp_err.h"
#include "commandMng.h"
#include "log.h"
#include "board.h"
#include "wifi.h"

static const char *TAG = "COMMAND_MNG";

#if defined(BOARD_MASTER)
void handle_frame_master(const proto_frame_t *frame)
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
        default: {
            break;
        }
    }
}
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
proto_frame_t *handle_frame_slave(const proto_frame_t *frame)
{
    static proto_frame_t response = { 0 };
    memset(&response, 0, sizeof(proto_frame_t));

    //log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d with len=%d from master", frame->header.cmd, frame->header.len);
    switch (frame->header.cmd) {
        case CMD_BOARD_STATUS: {
            board_status_t *board_status = getBoardStatus();
            proto_board_status_t board_status_pkt = { 0 };
            board_status_pkt.header.cmd = CMD_BOARD_STATUS_RESPONSE;
            board_status_pkt.header.len = sizeof(board_status_pkt.fields);
            board_status_pkt.fields.status = *board_status;
            memcpy(&response, &board_status_pkt, sizeof(proto_board_status_t));
            return &response;
        }
        case CMD_WIFI_CONFIG: {
            proto_wifi_config_t *wifi_config_frame = (proto_wifi_config_t *)frame;
            esp_err_t err = wifi_set_config(&wifi_config_frame->fields.wifi_config_ap, &wifi_config_frame->fields.wifi_config_sta, wifi_config_frame->fields.wifi_mode);
            proto_wifi_config_t wifi_config_response_frame = { 0 };
            wifi_config_response_frame.header.cmd  = CMD_WIFI_CONFIG_RESPONSE;
            wifi_config_response_frame.header.len = sizeof(wifi_config_response_frame.fields);
            memcpy(&wifi_config_frame->fields, &wifi_config_response_frame.fields, sizeof(wifi_config_response_frame.fields));
            wifi_config_response_frame.fields.status = err;
            memcpy(&response, &wifi_config_response_frame, sizeof(proto_wifi_config_t));
            return &response;
        }
        case CMD_WIFI_CHANNEL: {
            proto_wifi_set_channel_t *wifi_set_ch = (proto_wifi_set_channel_t *)frame;
            esp_err_t err = wifi_set_channel(wifi_set_ch->fields.channel);
            proto_wifi_set_channel_t wifi_set_ch_resp = { 0 };
            wifi_set_ch_resp.header.cmd = CMD_WIFI_CHANNEL_RESPONSE;
            wifi_set_ch_resp.header.len = sizeof(wifi_set_ch_resp.fields);
            wifi_set_ch_resp.fields.status = err;
            memcpy(&response, &wifi_set_ch_resp, sizeof(proto_wifi_set_channel_t));
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