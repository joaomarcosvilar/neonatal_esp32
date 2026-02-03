#include "assistant/i2cdev.h"
#include "aht.h"
#include "humidity.h"
#include "comon.h"

#define TAG "HUMIDITY"

static aht_t g_humidity_dev = {0};

esp_err_t humidity_init(void)
{
    esp_err_t res = i2cdev_init();
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init i2c channel (E: %s)", esp_err_to_name(res));
        return res;
    }

#ifndef TEST_NO_SENSORS
    g_humidity_dev.mode = AHT_MODE_NORMAL;
    g_humidity_dev.type = AHT_TYPE_AHT1x;

    res = aht_init_desc(&g_humidity_dev, HUMIDITY_SENSOR_ADDR, HUMIDITY_I2C_PORT, HUMIDITY_I2C_SDA_GPIO, HUMIDITY_I2C_SCL_GPIO);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init aht device descriptor (E: %s)", esp_err_to_name(res));
        return res;
    }

    res = aht_init(&g_humidity_dev);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init aht device (E: %s)", esp_err_to_name(res));
        return res;
    }

    bool calibration;
    res = aht_get_status(&g_humidity_dev, NULL, &calibration);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get status device (E: %s)", esp_err_to_name(res));
        return res;
    }
    if (!calibration)
    {
        ESP_LOGW(TAG, "Device not calibrated");
    }
#endif

    return ESP_OK;
}

float humidity_get(void)
{
    float humidity = 0.0;

#ifndef TEST_NO_SENSORS
    esp_err_t res = aht_get_data(&g_humidity_dev, NULL, &humidity);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get humidity (E: %s)", esp_err_to_name(res));
        return 0.0;
    }
#else 
    uint32_t raw = esp_random();
    humidity = (raw % (100 - 70 + 1)) + 70;
#endif
    return humidity;
}