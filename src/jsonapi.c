#include "cJSON.h"
#include "board.h"
#include "spi_proto.h"


static void add_board_status_json(cJSON *parent, const char *name, const board_status_t *st) 
{
    if (!parent || !name) return;
    if (!st) {
        cJSON_AddItemToObject(parent, name, cJSON_CreateNull());
        return;
    }
    cJSON *o = cJSON_CreateObject();
    cJSON_AddItemToObject(parent, name, o);
    cJSON *chip = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "chip", chip);
    cJSON_AddNumberToObject(chip, "model",   st->chip_model);     // vedi esp_chip_model_t
    cJSON_AddNumberToObject(chip, "cores",   st->chip_cores);
    cJSON_AddNumberToObject(chip, "revision",st->chip_revision);
    cJSON_AddNumberToObject(chip, "features",st->chip_features);  // bitmask
    cJSON *ram = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "ram", ram);
    cJSON_AddNumberToObject(ram, "total_internal",   st->total_internal_memory);
    cJSON_AddNumberToObject(ram, "free_internal",    st->free_internal_memory);
    cJSON_AddNumberToObject(ram, "largest_contig",   st->largest_contig_internal_block);
    cJSON_AddNumberToObject(ram, "spiram_size",      (double)st->spiram_size);
    cJSON *modules = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "modules", modules);
    cJSON_AddBoolToObject(modules, "spi",       st->spi_status != 0);
    cJSON_AddBoolToObject(modules, "netif",     st->netif_status != 0);
    cJSON_AddBoolToObject(modules, "wifi_init", st->wifi_init != 0);
    cJSON_AddBoolToObject(modules, "wifi_started", st->wifi_started != 0);
    cJSON_AddBoolToObject(modules, "bluetooth", st->bluetooth_status != 0);
    cJSON *wifi = cJSON_CreateObject();
    cJSON_AddItemToObject(o, "wifi", wifi);
    cJSON_AddStringToObject(wifi, "ap_ssid",     (char*)st->wifi_config_ap.ssid);
    cJSON_AddStringToObject(wifi, "ap_password", (char*)st->wifi_config_ap.password);
    cJSON_AddNumberToObject(wifi, "ap_channel",  st->wifi_config_ap.channel);
    cJSON_AddNumberToObject(wifi, "mode",        st->wifi_mode);
}

char *json_get_board_status(void)
{
    cJSON *root = cJSON_CreateObject();
    #if defined(BOARD_MASTER)
    board_status_t *master = getBoardStatus();
    board_status_t *slave1 = getSlaveStatus(ESPWROOM32);
    board_status_t *slave2 = getSlaveStatus(ESP32C5);
    board_status_t *slave3 = getSlaveStatus(ESP32S3);
    add_board_status_json(root, "master", master);
    add_board_status_json(root, "slave1", slave1);
    add_board_status_json(root, "slave2", slave2);
    add_board_status_json(root, "slave3", slave3);
    #elif defined(BOARD_SLAVE1)
    board_status_t *slave = getBoardStatus();
    add_board_status_json(root, "slave1", slave);
    #elif defined(BOARD_SLAVE2)
    board_status_t *slave1 = getBoardStatus();
    add_board_status_json(root, "slave1", slave);
    #elif defined(BOARD_SLAVE3)
    board_status_t *slave = getBoardStatus();
    add_board_status_json(root, "slave3", slave);
    #endif
    char *output = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return output;
}