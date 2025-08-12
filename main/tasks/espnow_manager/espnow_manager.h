#ifndef ESPNOW_MANEGER_H
#define ESPNOW_MANEGER_H

#define ESPNOW_CHANNEL 1

#define ESPNOW_FILE "espnow_addr.dat"
#define ESPNOW_FULL_PATH "/storage/espnow_addr.dat"

#include "comon.h"

esp_err_t espnow_manager_init(void);
esp_err_t espnow_manager_send(uint8_t *data, uint32_t len);
esp_err_t espnow_get_data(app_data_sensors_t *data);

#endif