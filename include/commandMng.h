#ifndef COMMANDMNG_H
#define COMMANDMNG_H

#include "spi_proto.h"

void handle_frame_master(proto_frame_t *frame);

proto_frame_t *handle_frame_slave(const proto_frame_t *frame);

/**
 * @brief Command used to send Wifi configuration to slave device
 * @param addr Slave address (index 1 based)
 * @param config_ap Wifi AP config
 * @param config_sta Wifi STA config
 * @param mode Wifi mode
 */
esp_err_t CommandSetWifiConfig(int addr, ap_config_t *config_ap, sta_config_t *config_sta, uint8_t mode);

/**
 * @brief Command used to set Wifi channel on slave device
 * @param addr Slave address (index 1 based)
 * @param channel Wifi channel to set (1 - 14)
 */
esp_err_t CommandSetWifiChannel(int addr, uint8_t channel);

/**
 * @brief Command used to start Wifi scan on slave device
 * @param addr Slave address (index 1 based)
 * @param scan_config Wifi scan configuration
 */
esp_err_t CommandWifiScan(int addr, scan_config_t *scan_config);

/**
 * @brief Command used to get Wifi scan results from slave device
 * @param addr Slave address (index 1 based)
 * @param in_results Pointer to store scan results - scan_results_t
 */
esp_err_t CommandWifiScanResults(int addr, scan_results_t *in_results);

#endif // COMMANDMNG_h