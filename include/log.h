#ifndef LOG_H
#define LOG_H

#define LOG_QUEUE_LENGTH 10

typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
} log_level_t;

typedef struct {
    char message[128];
} log_message_t;

/**
 * @brief Initialize the logging system with a specific log level.
 */
esp_err_t log_init(log_level_t level);

/**
 * @brief Set the current logging level.
 */
void log_set_level(log_level_t level);

/**
 * @brief Get the current logging level.
 */
log_level_t log_get_level();

/**
 * @brief Log a message with a specific log level and tag.
 */
void log_message(log_level_t level, const char *tag, const char *format, ...);


#endif // LOG_H