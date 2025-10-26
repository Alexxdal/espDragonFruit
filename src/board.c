#include <assert.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "esp_check.h"
#include "board.h"
#include "spi_proto.h"
#include "wifi.h"
#include "log.h"
#include "netif.h"
#include "nvs_config.h"
#include "esp_heap_caps.h"

#if defined(BOARD_MASTER)
#include "esp_spiffs.h"
#include "httpd.h"
#endif

static const char *TAG = "BOARD";
static TaskHandle_t common_board_task_handle;
static board_status_t board_status;
static SemaphoreHandle_t board_status_mtx;

#if defined(BOARD_MASTER)
    #if defined(HAS_PSRAM)
    EXT_RAM_BSS_ATTR static board_status_t slave_status[SLAVE_NUM];
    EXT_RAM_BSS_ATTR static scan_results_t slave_wifi_scan_results[SLAVE_NUM];
    #else
    static board_status_t slave_status[SLAVE_NUM] = { 0 };
    static scan_results_t slave_wifi_scan_results[SLAVE_NUM] = { 0 };
    #endif
#endif

#if defined(HAS_PSRAM)
EXT_RAM_BSS_ATTR static scan_results_t current_board_wifi_scan_results;
#else
static scan_results_t current_board_wifi_scan_results = { 0 };
#endif

static void common_board_task(void *arg)
{
    (void)arg;
    /* Get Chip info */
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    set_board_status {
        board_status.chip_cores = chip.cores;
        board_status.chip_features = chip.features;
        board_status.chip_model = chip.model;
        board_status.chip_revision = chip.revision;
        board_status.total_internal_memory = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        board_status.largest_contig_internal_block = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
        board_status.free_internal_memory = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        board_status.spiram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    }

    while(1)
    {
        set_board_status {
            board_status.free_internal_memory = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

#if defined(BOARD_MASTER)
static esp_err_t master_init()
{
    esp_err_t err = ESP_OK;

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "spiffs",
        .max_files = 15,
        .format_if_mount_failed = false
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    //TODO: Add this config in the spiffs like factory settings
    ap_config_t wifi_config_ap = {
        .ssid = "espDragonFruit",
        .password = "espDragonFruit",
        .channel = 1,
        .authmode = WIFI_AUTH_WPA2_PSK,
        .beacon_interval = 300,
        .max_connection = 5,
        .pmf_required = false,
        .pmf_capable = false
    };

    err = wifi_set_config(&wifi_config_ap, NULL, WIFI_MODE_AP);
    ESP_RETURN_ON_ERROR(err, TAG, "wifi_set_config");

    err = httpd_server_start();
    ESP_RETURN_ON_ERROR(err, TAG, "httpd_server_start");

    log_message(LOG_LEVEL_INFO, TAG, "Master board initialized.");
    return err;
}
#elif defined(BOARD_SLAVE1)
static esp_err_t slave_one_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave One board initialized.");
    return ESP_OK;
}
#elif defined(BOARD_SLAVE2)
static esp_err_t slave_two_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave Two board initialized.");
    return ESP_OK;
}
#elif defined(BOARD_SLAVE3)
static esp_err_t slave_three_init()
{
    log_message(LOG_LEVEL_INFO, TAG, "Slave Three board initialized.");
    return ESP_OK;
}
#endif

esp_err_t board_init(void)
{
    log_init(LOG_LEVEL_DEBUG);

    memset(&board_status, 0, sizeof(board_status_t));
    board_status_mtx = xSemaphoreCreateMutex();
    if(board_status_mtx == NULL) {
        log_message(LOG_LEVEL_ERROR, TAG, "Failed to create board status mutex");
        return ESP_ERR_NO_MEM;
    }

    ESP_ERROR_CHECK(nvs_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(network_interface_init());

#if defined(BOARD_MASTER)
    ESP_ERROR_CHECK(master_init());

#elif defined(BOARD_SLAVE1)
    ESP_ERROR_CHECK(slave_one_init());

#elif defined(BOARD_SLAVE2)
    ESP_ERROR_CHECK(slave_two_init());

#elif defined(BOARD_SLAVE3)
    ESP_ERROR_CHECK(slave_three_init());

#else
#   error "Define exactly one of: BOARD_MASTER, BOARD_SLAVE1, BOARD_SLAVE2, BOARD_SLAVE3"
#endif
    ESP_ERROR_CHECK(spi_proto_init());
    /* Start common board task */
    xTaskCreate(common_board_task, "common_board_task", 2048, NULL, 5, &common_board_task_handle);

    return ESP_OK;
}

board_status_t *getBoardStatus(void)
{
    return &board_status;
}

void board_status_lock(void) {
     xSemaphoreTake(board_status_mtx, portMAX_DELAY); 
}

void board_status_unlock(void) { 
    xSemaphoreGive(board_status_mtx); 
}

scan_results_t *getCurrentBoardWifiScanResults(void)
{
    return &current_board_wifi_scan_results;
}

esp_err_t setCurrentBoardWifiScanResults(scan_results_t *results)
{
    if(!results)
        return ESP_ERR_INVALID_ARG;

    memcpy(&current_board_wifi_scan_results, results, sizeof(scan_results_t));
    return ESP_OK;
}

#if defined(BOARD_MASTER)
board_status_t *getSlaveStatus(int addr)
{
    if(addr > (SLAVE_NUM-1) || addr < 0)
        return NULL;

    return &slave_status[addr];
}

scan_results_t *getSlaveWifiScanResults(int addr)
{
    if(addr > (SLAVE_NUM-1) || addr < 0)
        return NULL;

    return &slave_wifi_scan_results[addr];
}

esp_err_t setSlaveWifiScanResults(int addr, scan_results_t *results)
{
    if(addr > (SLAVE_NUM-1) || addr < 0)
        return ESP_ERR_INVALID_ARG;

    memcpy(&slave_wifi_scan_results[addr], results, sizeof(scan_results_t));
    return ESP_OK;
}
#endif