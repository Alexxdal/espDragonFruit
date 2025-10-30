#ifndef HTTPD_H
#define HTTPD_H

/**
 * @brief Start the control page http server
 */
esp_err_t httpd_server_start(void);

/**
 * @brief Stops the web server
 * Deallocates memory/resources used by an HTTP server instance and deletes it.
 */
esp_err_t httpd_server_stop(void);

#endif