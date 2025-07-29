#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#define TEMPERATURE_GPIO GPIO_NUM_5

#define TEMPERATURE_FILE "tempeature.dat"
#define TEMPERATURE_FULL_PATH "/storage/tempeature.dat"
#define TEMPERATURE_MAX_SENSOR_COUNT 10

esp_err_t temperature_init(void);
esp_err_t temperature_scan(void);
float temperature_get(uint8_t addr);

#endif