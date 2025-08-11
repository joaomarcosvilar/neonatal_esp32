#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "application.h"
#include "espnow_manager/espnow_manager.h"
#include "hardware/files/fs_manager.h"
#include "hardware/humidity/humidity.h"
#include "hardware/temperature/temperature.h"
#include "humidifier/humidifier.h"
#include "resistance/resistance.h"

#define TAG "APP"

#define APP_TASK_NAME "app task"
#define APP_TASK_STACK_SIZE 1024 * 4
#define APP_TASK_PRIOR 2

#define APP_SEMAPHORE_TIMEOUT_TICKS 5000
#define APP_QUEUE_LEN 10

#define APP_TIMER_NAME "timer send"
#define APP_TIMER_PERIOD_MS 1000 * 1
#define APP_TIMER_AUTO_RELOAD pdTRUE

#define APP_EVENT_SEND (1 << 0)
#define APP_EVENT_RECEIVED (1 << 1)

static QueueHandle_t app_queue;
static TaskHandle_t app_task_handle;
static EventGroupHandle_t app_event;
TimerHandle_t app_timer;

void app_timer_cb(TimerHandle_t app_timer)
{
    xEventGroupSetBits(app_event, APP_EVENT_SEND);
}

void app_task(void *args)
{
    app_data_actuator_t actuator = {0};
    app_data_sensors_t sensor = {0};
    EventBits_t event;
    esp_err_t ret = ESP_OK;
    for (;;)
    {
        event = xEventGroupWaitBits(app_event, APP_EVENT_SEND | APP_EVENT_RECEIVED, pdTRUE, pdFALSE, pdMS_TO_TICKS(100));
        if (event & APP_EVENT_SEND)
        {
            ESP_LOGI(TAG, "APP_EVENT_SEND");
            ret = temperature_get_all(&sensor.temp);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to get temperatures value");
            }

            // for (uint8_t i = 0; i < TEMPERATURE_MAX_SENSOR_COUNT; i++)
            // {
            //     ESP_LOGI(TAG, "temp sensor [%d]: %.2f", i, sensor.temp[i]);
            // }

            sensor.hum = humidity_get();
            // ESP_LOGI(TAG, "hum sensor: %.2f", sensor.hum);

            ret = espnow_manager_send((uint8_t *)&sensor, sizeof(app_data_sensors_t));
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to send espnow");
            }
        }

        if (xQueueReceive(app_queue, &actuator, pdMS_TO_TICKS(100)) == pdPASS)
        {
            ESP_LOGI(TAG, "Received:\n perc_res=%d\npwm_hum=%d", actuator.perc_res, actuator.pwm_hum);
            ret = resistance_set(actuator.perc_res);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to set percentage power resistence (E: %s)", esp_err_to_name(ret));
            }

            ret = humidifier_set(actuator.pwm_hum);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to set percentage pwm humidifier (E: %s)", esp_err_to_name(ret));
            }
        }
    }
}

esp_err_t application_init(void)
{
    app_queue = xQueueCreate(APP_QUEUE_LEN, sizeof(app_data_actuator_t));
    if (app_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queue");
        return ESP_FAIL;
    }

    app_event = xEventGroupCreate();
    if (app_event == NULL)
    {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    app_timer = xTimerCreate(APP_TIMER_NAME,
                             pdMS_TO_TICKS(APP_TIMER_PERIOD_MS),
                             APP_TIMER_AUTO_RELOAD,
                             (void *)0,
                             app_timer_cb);
    if (app_timer == NULL)
    {
        ESP_LOGE(TAG, "Failed to create timer");
        return ESP_FAIL;
    }
    xTimerStart(app_timer, 0);

    BaseType_t xRet = xTaskCreate(
        app_task,
        APP_TASK_NAME,
        APP_TASK_STACK_SIZE,
        NULL,
        APP_TASK_PRIOR,
        &app_task_handle);
    if (xRet != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to create task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Aplication init");
    return ESP_OK;
}

esp_err_t app_set(app_data_actuator_t set)
{
    if (xQueueSend(app_queue, &set, 0) != pdPASS)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}