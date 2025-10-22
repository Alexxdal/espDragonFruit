#include <stdio.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "log.h"

static log_level_t current_log_level = LOG_LEVEL_VERBOSE;

void log_set_level(log_level_t level) {
    current_log_level = level;
}

log_level_t log_get_level() {
    return current_log_level;
}

void log_message(log_level_t level, const char *tag, const char *format, ...) 
{    
    /* TODO: Do not direct printf in ISR, do a queue */
    if (xPortInIsrContext()) {
        return;
    }

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

    va_list args;
    va_start(args, format);
    printf("[%s][%s] %s: ", board_type, level_str, tag);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}