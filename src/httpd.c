#if defined(BOARD_MASTER)
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "httpd.h"
#include "cJSON.h"
#include "utility.h"
#include "wifi.h"
#include "board.h"
#include "spi_proto.h"
#include "log.h"
#include "jsonapi.h"
#include "commandMng.h"

static const char *TAG = "HTTPD";
static httpd_handle_t server;

static const char* mime_from_path(const char* path) {
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".css"))  return "text/css";
    if (strstr(path, ".js"))   return "application/javascript";
    if (strstr(path, ".png"))  return "image/png";
    if (strstr(path, ".jpg"))  return "image/jpeg";
    if (strstr(path, ".ico"))  return "image/x-icon";
    if (strstr(path, ".svg"))  return "image/svg+xml";
    return "text/plain";
}

static esp_err_t cors_prevention_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_get_handler(httpd_req_t *req)
{
    char *json = json_get_board_status();
    if (!json) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "cJSON print failed");
        return ESP_OK;
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin",  "*");
    httpd_resp_set_type(req, "application/json");
    esp_err_t r = httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    return r;
}

static esp_err_t file_get_handler(httpd_req_t *req) 
{
    char filepath[128] = "/spiffs";
    const char *uri = req->uri;

    if (strcmp(uri, "/") == 0) 
        uri = "/index.html";

    if (strcmp(uri, "/status") == 0)
        return status_get_handler(req);

    strlcat(filepath, uri, sizeof(filepath));

    FILE *f = fopen(filepath, "r");
    if (!f) 
    { 
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found");
        return ESP_OK; 
    }

    httpd_resp_set_type(req, mime_from_path(filepath));
    
    char buf[1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) 
    {
        if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) 
        { 
            fclose(f); 
            return ESP_FAIL; 
        }
    }
    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t wifi_scan_get_handler(httpd_req_t *req) 
{
    char query[64] = {0};
    char device[16] = {0};

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "device", device, sizeof(device));
    }
    if (device[0] == '\0') {
        strcpy(device, "master");
    }

    int target_device = 0;
    if      (strcmp(device, "master") == 0) target_device = 255;
    else if (strcmp(device, "slave1") == 0) target_device = ESPWROOM32;
    else if (strcmp(device, "slave2") == 0) target_device = ESP32C5;
    else if (strcmp(device, "slave3") == 0) target_device = ESP32S3;
    else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid device");
        return ESP_OK;
    }

    board_status_t *st = NULL;
    if(target_device == 255) {
        st = getBoardStatus();
    } else {
        st = getSlaveStatus(target_device);
    }

    cJSON *out = cJSON_CreateObject();
    if (!out) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON alloc failed");
        return ESP_OK;
    }

    if (st->wifi_scan_started && !st->wifi_scan_done) 
    {
        cJSON_AddStringToObject(out, "state", "scanning");
    } 
    else if (!st->wifi_scan_started && !st->wifi_scan_done) 
    {
        cJSON_AddStringToObject(out, "state", "idle");
    } 
    else if (st->wifi_scan_done) 
    {
        cJSON_AddStringToObject(out, "state", "done");
        cJSON_AddNumberToObject(out, "error", st->wifi_scan_error);

        scan_results_t *results = NULL;
        if(target_device == 255) {
            results = getCurrentBoardWifiScanResults();
        } else {
            results = getSlaveWifiScanResults(target_device);
        }

        if (results && results->ap_num > 0) 
        {
            cJSON_AddNumberToObject(out, "ap_num", results->ap_num);
            cJSON *aps = cJSON_CreateArray();
            cJSON_AddItemToObject(out, "results", aps);

            for (int i = 0; i < results->ap_num; ++i) {
                scan_result_t *ap_res = &results->results[i];
                cJSON *ap = cJSON_CreateObject();
                if (!ap) {
                    continue;
                }
                cJSON_AddItemToArray(aps, ap);

                // Base
                cJSON_AddStringToObject(ap, "ssid", (char*)ap_res->ssid);
                cJSON_AddNumberToObject(ap, "rssi", ap_res->rssi);
                cJSON_AddNumberToObject(ap, "primary_channel",   ap_res->primary_channel);
                cJSON_AddNumberToObject(ap, "secondary_channel", ap_res->secondary_channel);

                char bssid_str[18];
                snprintf(bssid_str, sizeof(bssid_str),
                        "%02X:%02X:%02X:%02X:%02X:%02X",
                        ap_res->bssid[0], ap_res->bssid[1], ap_res->bssid[2],
                        ap_res->bssid[3], ap_res->bssid[4], ap_res->bssid[5]);
                cJSON_AddStringToObject(ap, "bssid", bssid_str);
                cJSON_AddStringToObject(ap, "authmode", wifi_auth_mode_to_str(ap_res->authmode));
                cJSON_AddStringToObject(ap, "pairwise_cipher", wifi_cipher_type_to_str(ap_res->pairwise_cipher));
                cJSON_AddStringToObject(ap, "group_cipher", wifi_cipher_type_to_str(ap_res->group_cipher));
                cJSON_AddBoolToObject(ap, "wps", ap_res->wps ? true : false);
                // PHY / capacità radio
                cJSON_AddNumberToObject(ap, "phy_11b",  ap_res->phy_11b);
                cJSON_AddNumberToObject(ap, "phy_11g",  ap_res->phy_11g);
                cJSON_AddNumberToObject(ap, "phy_11n",  ap_res->phy_11n);
                cJSON_AddNumberToObject(ap, "phy_11a",  ap_res->phy_11a);
                cJSON_AddNumberToObject(ap, "phy_11ac", ap_res->phy_11ac);
                cJSON_AddNumberToObject(ap, "phy_11ax", ap_res->phy_11ax);
                cJSON_AddNumberToObject(ap, "phy_lr",   ap_res->phy_lr);
                cJSON_AddNumberToObject(ap, "ant",      ap_res->ant);

                // Bandwidth / VHT
                /*cJSON_AddNumberToObject(ap, "bandwidth",    ap_res->bandwidth);
                cJSON_AddNumberToObject(ap, "vht_ch_freq1", ap_res->vht_ch_freq1);
                cJSON_AddNumberToObject(ap, "vht_ch_freq2", ap_res->vht_ch_freq2);*/

                // FTM (location / ranging)
                cJSON_AddNumberToObject(ap, "ftm_responder", ap_res->ftm_responder);
                cJSON_AddNumberToObject(ap, "ftm_initiator", ap_res->ftm_initiator);

                // Country / regolatorio
                char country[4] = {0};
                memcpy(country, ap_res->country_code, 3);
                cJSON_AddStringToObject(ap, "country_code", country);
                cJSON_AddNumberToObject(ap, "schan",        ap_res->schan);
                cJSON_AddNumberToObject(ap, "nchan",        ap_res->nchan);
                cJSON_AddNumberToObject(ap, "max_tx_power", ap_res->max_tx_power);
                cJSON_AddNumberToObject(ap, "policy",       ap_res->policy);
                cJSON_AddNumberToObject(ap, "wifi_5g_channel_mask",
                                        ap_res->wifi_5g_channel_mask);

                // HE / BSS color
                /*cJSON_AddNumberToObject(ap, "bss_color",          ap_res->bss_color);
                cJSON_AddNumberToObject(ap, "partial_bss_color", ap_res->partial_bss_color);
                cJSON_AddNumberToObject(ap, "bss_color_disabled", ap_res->bss_color_disabled);
                cJSON_AddNumberToObject(ap, "bssid_index",        ap_res->bssid_index);*/
            }
        }
    }

    char *json_str = cJSON_PrintUnformatted(out);
    cJSON_Delete(out);
    if (!json_str) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON print failed");
        return ESP_OK;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    esp_err_t resp_err = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
    free(json_str);
    return resp_err;
}

static esp_err_t wifi_scan_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    int ret;
    int scan_start_timeout = 5000 / 10;

    char target[16] = {0};
    int target_device = 0;
    scan_config_t scan_config = {
        .channel = 0,
        .show_hidden = 1,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = 120,
    };

    if (total_len <= 0 || total_len > 1024) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid body");
        return ESP_OK;
    }
    char *buf = malloc(total_len + 1);
    if (!buf) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No mem");
        return ESP_OK;
    }

    while (cur_len < total_len) {
        ret = httpd_req_recv(req, buf + cur_len, total_len - cur_len);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            free(buf);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Recv error");
            return ESP_OK;
        }
        cur_len += ret;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        free(buf);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_OK;
    }

    cJSON *j_target   = cJSON_GetObjectItem(root, "device");
    cJSON *j_scantime = cJSON_GetObjectItem(root, "scantime");
    cJSON *j_channel  = cJSON_GetObjectItem(root, "channel");
    cJSON *j_passive  = cJSON_GetObjectItem(root, "passive");
    if (cJSON_IsString(j_target) && (j_target->valuestring != NULL)) {
        strlcpy(target, j_target->valuestring, sizeof(target));
    }
    if (cJSON_IsNumber(j_scantime)) {
        scan_config.scan_time = j_scantime->valueint; // se lo mandi in secondi
    }
    if (cJSON_IsNumber(j_channel)) {
        scan_config.channel = j_channel->valueint;
    }
    if (cJSON_IsBool(j_passive)) {
        scan_config.scan_type = cJSON_IsTrue(j_passive) ? WIFI_SCAN_TYPE_PASSIVE : WIFI_SCAN_TYPE_ACTIVE;
    }
    cJSON_Delete(root);
    free(buf);

    if(strcmp(target, "master") == 0) {
        target_device = 255;
    } else if(strcmp(target, "slave1") == 0) {
        target_device = ESPWROOM32;
    } else if(strcmp(target, "slave2") == 0) {
        target_device = ESP32C5;
    } else if(strcmp(target, "slave3") == 0) {
        target_device = ESP32S3;
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid device");
        return ESP_OK;
    }

    if(target_device == 255) {
        if(getBoardStatus()->wifi_scan_started == true) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Scan already started.");
            return ESP_OK;
        }
        esp_err_t err = wifi_scan(&scan_config);
        if (err != ESP_OK) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to start WiFi scan");
            return ESP_OK;
        }
        while(getBoardStatus()->wifi_scan_started != true) {
            vTaskDelay(pdMS_TO_TICKS(10));
            if(scan_start_timeout <= 0) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Scan start timeout.");
                return ESP_OK;
            }
            scan_start_timeout--;
        }
    }
    else {
        if(getSlaveStatus(target_device)->wifi_scan_started == true) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Scan already started.");
            return ESP_OK;
        }
        esp_err_t err = CommandWifiScan(target_device, &scan_config);
        if (err != ESP_OK) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to start WiFi scan.");
            return ESP_OK;
        }
        while(getSlaveStatus(target_device)->wifi_scan_started != true) {
            vTaskDelay(pdMS_TO_TICKS(10));
            if(scan_start_timeout <= 0) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Scan start timeout.");
                return ESP_OK;
            }
            scan_start_timeout--;
        }
    }

    cJSON *out = cJSON_CreateObject();
    if (!out) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON alloc failed");
        return ESP_OK;
    }

    cJSON_AddStringToObject(out, "status", "started");
    char *json_str = cJSON_PrintUnformatted(out);
    cJSON_Delete(out);

    if (!json_str) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON print failed");
        return ESP_OK;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    esp_err_t resp_err = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
    free(json_str);
    return resp_err;
}

esp_err_t httpd_server_start(void) 
{
    if(server != NULL) {
        log_message(LOG_LEVEL_DEBUG, TAG, "httpd server already started.");
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t err = ESP_OK;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_uri_t api = { 0 };
    if ((err = httpd_start(&server, &config)) == ESP_OK) 
    {
        api.uri = "/api/wifi_scan"; 
        api.method = HTTP_GET;
        api.handler = wifi_scan_get_handler; 
        api.user_ctx = NULL;
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &api));

        api.uri = "/api/wifi_scan"; 
        api.method = HTTP_POST;
        api.handler = wifi_scan_post_handler; 
        api.user_ctx = NULL;
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &api));

        httpd_uri_t any = {
            .uri = "/*", 
            .method = HTTP_GET, 
            .handler = file_get_handler, 
            .user_ctx = NULL
        };
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &any));

        httpd_uri_t cors_prevention = {
            .uri      = "/*",
            .method   = HTTP_OPTIONS,
            .handler  = cors_prevention_handler,
            .user_ctx = NULL
        };
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &cors_prevention));

        log_message(LOG_LEVEL_INFO, TAG, "httpd server started.");
    }
    return err;
}

esp_err_t httpd_server_stop(void)
{
    if(server == NULL)
    {
        log_message(LOG_LEVEL_DEBUG, TAG, "httpd server not started.");
        return ESP_ERR_INVALID_ARG;
    }
    return httpd_stop(&server);
}
#endif // BOARD_MASTER