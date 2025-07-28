#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "driver/ledc.h"
#include "humidifier.h"

#define TAG "HUMIDIFIER"

esp_err_t humidifier_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};

    esp_err_t res = ledc_timer_config(&ledc_timer);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed in config timer ledc");
        return res;
    }

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 0,
        .hpoint = 0};

    res = ledc_channel_config(&ledc_channel);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init channel ledc");
        return res;
    }

    return ESP_OK;
}

esp_err_t humidifier_set(uint16_t percent)
{
    if (percent > 100)
        percent = 100;

    uint32_t duty = (uint32_t)((2 ^ LEDC_DUTY_RES) * percent / 100.0f);

    esp_err_t res = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    if (res != ESP_OK)
    {
        return res;
    }

    res = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    return res;
}
