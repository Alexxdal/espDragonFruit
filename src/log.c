#include <stdio.h>
#include <esp_log.h>
#include "log.h"

static log_level_t current_log_level = LOG_LEVEL_DEBUG;

void log_set_level(log_level_t level) {
    current_log_level = level;
}

log_level_t log_get_level() {
    return current_log_level;
}

void log_message(log_level_t level, const char *tag, const char *format, ...) {
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

    va_list args;
    va_start(args, format);
    printf("[%s] %s: ", level_str, tag);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}