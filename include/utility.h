#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "wifi.h"

wifi_config_t *wifi_convert_sta_config(sta_config_t *config_sta);

wifi_config_t *wifi_convert_ap_config(ap_config_t *config_ap);

const char *wifi_auth_mode_to_str(uint8_t authmode);

const char *wifi_cipher_type_to_str(uint8_t cipher);

#endif /* _UTILITY_H_ */