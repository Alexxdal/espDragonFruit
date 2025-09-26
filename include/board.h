#ifndef BOARD_H
#define BOARD_H

#pragma once
#include "esp_err.h"

/**
 * @brief Initialize the board based on its type (master or slave).
 */
esp_err_t board_init();

#endif // BOARD_H