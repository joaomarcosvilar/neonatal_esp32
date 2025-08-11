#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#define TEMPERATURE_GPIO GPIO_NUM_4
#define TEMPERATURE_MAX_SENSOR_COUNT 10

#define TEMPERATURE_FILE "temperature.dat"
#define TEMPERATURE_FULL_PATH "/storage/temperature.dat"

esp_err_t temperature_init(void);
esp_err_t temperature_scan(void);
float temperature_get(uint8_t addr);
esp_err_t temperature_get_all(float *data);

#endif