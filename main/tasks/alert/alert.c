#include "driver/rmt.h"
#include "esp_log.h"
#include "stdio.h"
#include "driver/gpio.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "alert.h"

#define TAG "ALERT"

#define ALERT_TASK_NAME "alert task"
#define ALERT_TASK_STACK_SIZE (1024 * 2)
#define ALERT_TASK_PRIOR 3

#define ALERT_LED_MAX 1

#define ALERT_BIT_CLEAN (1 << 0)
#define ALERT_BIT_ESPNOW_FAIL (1 << 1)
#define ALERT_BIT_INIT_FAIL (1 << 2)
#define ALERT_BIT_FINISH (1 << 3)

#define ALERT_MAX 4
#define LED_MAX_BRIGHTNESS 10

static TaskHandle_t alert_task_handle = NULL;
static EventGroupHandle_t alert_event_group;
static led_strip_handle_t led_alert;

typedef struct
{
    uint8_t red, green, blue;
    uint32_t on_ms;
    uint32_t off_ms;
    uint32_t repeat;
} alert_cfg_t;

static const alert_cfg_t alert_cfgs[] = {
    [ALERT_CLEAN] = {0, 0, 0, 0, 0, 1},                                              // LED apagado
    [ALERT_ESPNOW_SEND_FAIL] = {LED_MAX_BRIGHTNESS, 0, 0, 200, 200, 2},              // Vermelho rápido
    [ALERT_INIT_FAIL] = {LED_MAX_BRIGHTNESS, LED_MAX_BRIGHTNESS, 0, 500, 500, 1000}, // Amarelo médio
    [ALERT_FINISH] = {0, LED_MAX_BRIGHTNESS, 0, 1000, 1000, 1},                      // Verde lento
};

static const EventBits_t alert_bits_map[] = {
    [ALERT_CLEAN] = ALERT_BIT_CLEAN,
    [ALERT_ESPNOW_SEND_FAIL] = ALERT_BIT_ESPNOW_FAIL,
    [ALERT_INIT_FAIL] = ALERT_BIT_INIT_FAIL,
    [ALERT_FINISH] = ALERT_BIT_FINISH,
};

static void alert_off(void)
{
    led_strip_clear(led_alert);
}

static void alert_task(void *arg)
{
    while (1)
    {
        EventBits_t bits = xEventGroupWaitBits(alert_event_group,
                                               ALERT_BIT_CLEAN | ALERT_BIT_ESPNOW_FAIL | ALERT_BIT_INIT_FAIL | ALERT_BIT_FINISH,
                                               pdTRUE, 
                                               pdFALSE,
                                               portMAX_DELAY);

        for (int i = 0; i < ALERT_MAX; i++)
        {
            if (bits & alert_bits_map[i])
            {
                alert_cfg_t cfg = alert_cfgs[i];
                for (uint32_t r = 0; r < cfg.repeat; r++)
                {
                    led_strip_set_pixel(led_alert, 0, cfg.red, cfg.green, cfg.blue);
                    led_strip_refresh(led_alert);
                    vTaskDelay(pdMS_TO_TICKS(cfg.on_ms));
                    alert_off();
                    vTaskDelay(pdMS_TO_TICKS(cfg.off_ms));
                }
            }
        }
    }
}

esp_err_t alert_set(alert_color_e alert)
{
    xEventGroupSetBits(alert_event_group, alert_bits_map[alert]);
    return ESP_OK;
}

esp_err_t alert_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = ALERT_LED_GPIO,
        .max_leds = ALERT_LED_MAX};

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };

    esp_err_t res = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_alert);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to config led strip (E:%s)", esp_err_to_name(res));
        return res;
    }

    alert_event_group = xEventGroupCreate();
    if (!alert_event_group)
    {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    BaseType_t xRet = xTaskCreate(alert_task,
                                  ALERT_TASK_NAME,
                                  ALERT_TASK_STACK_SIZE,
                                  NULL,
                                  ALERT_TASK_PRIOR,
                                  &alert_task_handle);

    if (xRet != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task");
        return ESP_FAIL;
    }

    alert_set(ALERT_CLEAN);

    return ESP_OK;
}
