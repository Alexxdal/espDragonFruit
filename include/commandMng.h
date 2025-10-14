#ifndef COMMANDMNG_H
#define COMMANDMNG_H

#include "spi_proto.h"

void handle_frame_master(const proto_frame_t *frame);

proto_frame_t *handle_frame_slave(const proto_frame_t *frame);

/**
 * @brief Command used to send Wifi configuration to slave device
 * @param addr Slave address (index 1 based)
 * @param config_ap Wifi AP config
 * @param config_sta Wifi STA config
 * @param mode Wifi mode
 */
esp_err_t CommandSetWifiConfig(int addr, ap_config_t *config_ap, sta_config_t *config_sta, uint8_t mode);

#endif // COMMANDMNG_h