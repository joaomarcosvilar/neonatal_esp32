#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "string.h"
#include "inttypes.h"
#include "driver/gpio.h"
#include "assistant/onewire.h"

#include "ds18x20.h"
#include "hardware/files/fs_manager.h"
#include "temperature.h"

#define TAG "TEMPERATURE"

static onewire_addr_t g_temperature_sensors_addr[TEMPERATURE_MAX_SENSOR_COUNT] = {0};
static size_t g_temperature_sensor_count = 0;

void temperature_address_print(void)
{
    for (int i = 0; i < g_temperature_sensor_count; i++)
        ESP_LOGI(TAG, "Sensor %08" PRIx32 "%08" PRIx32 "(%d)",
                 (uint32_t)(g_temperature_sensors_addr[i] >> 32), (uint32_t)g_temperature_sensors_addr[i], i + 1);
}

esp_err_t temperature_init(void)
{
    esp_err_t res = fs_search(ROOT_STORAGE_PATH, TEMPERATURE_FILE);
    if (res == ESP_ERR_NOT_FOUND)
    {
        temperature_scan();
    }
    else if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get addrs in file (E: %s)", esp_err_to_name(res));
        return res;
    }

    res = fs_read(TEMPERATURE_FULL_PATH, 0, sizeof(g_temperature_sensors_addr), (uint8_t *)g_temperature_sensors_addr);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed get sensor address (E: %s)", esp_err_to_name(res));
    }

    temperature_scan();

    return res;
}

esp_err_t temperature_scan(void)
{
    onewire_addr_t addrs_current[TEMPERATURE_MAX_SENSOR_COUNT];
    uint32_t count_current = 0;

    esp_err_t res = ds18x20_scan_devices(TEMPERATURE_GPIO,
                                         addrs_current,
                                         TEMPERATURE_MAX_SENSOR_COUNT,
                                         &count_current);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to scan address sensor (E: %s)", esp_err_to_name(res));
        return res;
    }

    if (count_current != g_temperature_sensor_count)
    {
        memcpy(g_temperature_sensors_addr, addrs_current, sizeof(addrs_current));
        g_temperature_sensor_count = count_current;
    }

    if (!g_temperature_sensor_count)
    {
        ESP_LOGE(TAG, "No sensores detected");
        return ESP_ERR_NOT_FOUND;
    }

    res = fs_write(TEMPERATURE_FULL_PATH,
                   0,
                   sizeof(g_temperature_sensors_addr),
                   (uint8_t *)g_temperature_sensors_addr,
                   SPIFFS_NW_W);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create file (E: %s)", esp_err_to_name(res));
    }

    temperature_address_print();
    return ESP_OK;
}

float temperature_get(uint8_t addr)
{
    if (addr > g_temperature_sensor_count || addr == 0)
    {
        ESP_LOGE(TAG, "Invalid sensor index: %u", addr);
        return 0.0;
    }

    float temp = 0.0;
    esp_err_t res = ds18x20_measure_and_read(TEMPERATURE_GPIO, g_temperature_sensors_addr[addr - 1], &temp);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Sensors read error (E: %s)", esp_err_to_name(res));
        return 0.0;
    }

    return temp;
}

esp_err_t temperature_get_all(float *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    float temps[TEMPERATURE_MAX_SENSOR_COUNT] = {0};

    esp_err_t ret = ds18x20_measure_and_read_multi(
        TEMPERATURE_GPIO,
        g_temperature_sensors_addr,
        g_temperature_sensor_count,
        temps
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read sensors (E: %s)", esp_err_to_name(ret));
        return ret;
    }

    memcpy(data, temps, sizeof(float) * g_temperature_sensor_count);

    return ESP_OK;
}
