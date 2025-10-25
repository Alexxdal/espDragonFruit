#include <stdio.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "log.h"

static log_level_t current_log_level = LOG_LEVEL_VERBOSE;
static QueueHandle_t log_queue;

static void log_task(void *arg) {
    (void)arg;
    log_message_t log_msg;
    while (1) {
        if (xQueueReceive(log_queue, &log_msg, portMAX_DELAY) == pdTRUE) {
            printf("%s\n", log_msg.message);
        }
    }
}

esp_err_t log_init(log_level_t level) {
    current_log_level = level;
    log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(log_message_t));
    if (log_queue == NULL) {
        printf("Failed to create log queue\n");
        return ESP_ERR_NO_MEM;
    }
    xTaskCreate(log_task, "log_task", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
    return ESP_OK;
}

void log_set_level(log_level_t level) {
    current_log_level = level;
}

log_level_t log_get_level() {
    return current_log_level;
}

void log_message(log_level_t level, const char *tag, const char *format, ...) 
{   
    log_message_t log_msg;
    if (level > current_log_level || level == LOG_LEVEL_NONE) {
        return;
    }

    const char *level_str;
    switch (level) {
        case LOG_LEVEL_ERROR:   level_str = "ERROR"; break;
        case LOG_LEVEL_WARN:    level_str = "WARN"; break;
        case LOG_LEVEL_INFO:    level_str = "INFO"; break;
        case LOG_LEVEL_DEBUG:   level_str = "DEBUG"; break;
        case LOG_LEVEL_VERBOSE: level_str = "VERBOSE"; break;
        default:                level_str = "UNKNOWN"; break;
    }

    #if defined(BOARD_MASTER)
    const char *board_type = "MASTER";
    #elif defined(BOARD_SLAVE1)
    const char *board_type = "SLAVE1";
    #elif defined(BOARD_SLAVE2)
    const char *board_type = "SLAVE2";
    #elif defined(BOARD_SLAVE3)
    const char *board_type = "SLAVE3";
    #else
    const char *board_type = "UNKNOWN";
    #endif

    int n = snprintf(log_msg.message, sizeof(log_msg.message), "[%s][%s] %s: ", board_type, level_str, (tag ? tag : ""));
    if (n < 0) { 
        n = 0;
    }
    if (n >= (int)sizeof(log_msg.message)) { 
        n = sizeof(log_msg.message) - 1;
    }

    va_list args;
    va_start(args, format);
    vsnprintf(log_msg.message + n, sizeof(log_msg.message) - n, format ? format : "", args);
    va_end(args);

    if (xPortInIsrContext() == pdTRUE) {
        if( log_queue != NULL) {
            xQueueSendFromISR(log_queue, &log_msg, NULL);
        }
    }
    else {
        if (log_queue != NULL) {
            xQueueSend(log_queue, &log_msg, 0);
        }
    }
}