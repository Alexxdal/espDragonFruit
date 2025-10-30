#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "httpd.h"
#include "board.h"
#include "spi_proto.h"
#include "log.h"
#include "jsonapi.h"

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

esp_err_t httpd_server_start(void) 
{
    if(server != NULL) {
        log_message(LOG_LEVEL_DEBUG, TAG, "httpd server already started.");
        return ESP_ERR_INVALID_STATE;
    }
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