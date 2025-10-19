#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "httpd.h"
#include "board.h"
#include "log.h"
#include "cJSON.h"

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

static void add_board_status_json(cJSON *parent, const char *name, const board_status_t *st) {
    if (!parent || !name) return;

    if (!st) {
        // Se non disponibile, metti null
        cJSON_AddItemToObject(parent, name, cJSON_CreateNull());
        return;
    }

    cJSON *o = cJSON_CreateObject();
    cJSON_AddItemToObject(parent, name, o);
    // Chip info
    cJSON *chip = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "chip", chip);
    cJSON_AddNumberToObject(chip, "model",   st->chip_model);     // vedi esp_chip_model_t
    cJSON_AddNumberToObject(chip, "cores",   st->chip_cores);
    cJSON_AddNumberToObject(chip, "revision",st->chip_revision);
    cJSON_AddNumberToObject(chip, "features",st->chip_features);  // bitmask
    // RAM
    cJSON *ram = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "ram", ram);
    cJSON_AddNumberToObject(ram, "total_internal",   st->total_internal_memory);
    cJSON_AddNumberToObject(ram, "free_internal",    st->free_internal_memory);
    cJSON_AddNumberToObject(ram, "largest_contig",   st->largest_contig_internal_block);
    cJSON_AddNumberToObject(ram, "spiram_size",      (double)st->spiram_size);
    // Module status
    cJSON *m = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "modules", m);
    cJSON_AddBoolToObject(m, "spi",       st->spi_status != 0);
    cJSON_AddBoolToObject(m, "netif",     st->netif_status != 0);
    cJSON_AddBoolToObject(m, "wifi_init", st->wifi_init != 0);
    cJSON_AddBoolToObject(m, "wifi_started", st->wifi_started != 0);
    cJSON_AddBoolToObject(m, "bluetooth", st->bluetooth_status != 0);
    // Wi-Fi
    cJSON *wifi = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "wifi", wifi);
    cJSON_AddStringToObject(wifi, "ap_ssid",     (char*)st->wifi_config_ap.ssid);
    cJSON_AddStringToObject(wifi, "ap_password", (char*)st->wifi_config_ap.password);
    cJSON_AddNumberToObject(wifi, "ap_channel",  st->wifi_config_ap.channel);
    cJSON_AddNumberToObject(wifi, "mode",        st->wifi_mode);
}

static esp_err_t status_get_handler(httpd_req_t *req)
{
    board_status_t *master = getBoardStatus();
    board_status_t *slave1 = getSlaveStatus(1);
    board_status_t *slave2 = getSlaveStatus(2);
    board_status_t *slave3 = getSlaveStatus(3);
    cJSON *root = cJSON_CreateObject();
    
    add_board_status_json(root, "master", master);
    add_board_status_json(root, "slave1", slave1);
    add_board_status_json(root, "slave2", slave2);
    add_board_status_json(root, "slave3", slave3);

    // Serializza (senza spazi)
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
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

esp_err_t httpd_server_start(void) 
{
    esp_err_t err = ESP_OK;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    if ((err = httpd_start(&server, &config)) == ESP_OK) 
    {
        httpd_uri_t any = {
            .uri = "/*", 
            .method = HTTP_GET, 
            .handler = file_get_handler, 
            .user_ctx = NULL
        };
        err = httpd_register_uri_handler(server, &any);
        if(err != ESP_OK)
            return err;

        log_message(LOG_LEVEL_INFO, TAG, "httpd server started");
    }
    return err;
}