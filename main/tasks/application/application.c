#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

#include "application.h"
#include "espnow_manager/espnow_manager.h"
#include "hardware/files/fs_manager.h"
#include "hardware/humidity/humidity.h"
#include "hardware/temperature/temperature.h"
#include "alert/alert.h"
#include "humidifier/humidifier.h"
#include "resistance/resistance.h"

#define TAG "APP"

#define APP_TASK_SEND_NAME "app send task"
#define APP_TASK_SEND_STACK_SIZE 1024 * 3
#define APP_TASK_SEND_PRIOR 1

#define APP_TASK_RECEIVED_NAME "app receive task"
#define APP_TASK_RECEIVED_STACK_SIZE 1024 * 3
#define APP_TASK_RECEIVED_PRIOR 2

#define APP_TIME_SEND_US 10

#define APP_QUEUE_LEN 10

static QueueHandle_t app_queue;
// static TaskHandle_t app_task_handle;

void app_send_task(void *args)
{

    app_data_sensors_t sensor = {0};
    esp_err_t ret = ESP_OK;
    uint32_t timer_current = esp_timer_get_time();
    for (;;)
    {
        if (esp_timer_get_time() - timer_current >= APP_TIME_SEND_US)
        {
            sensor.temp[0] = temperature_get(1);
            // for (uint8_t i = 0; i < TEMPERATURE_MAX_SENSOR_COUNT; i++)
            // {
            //     ESP_LOGI(TAG, "temp sensor [%d]: %.2f", 0, sensor.temp[0]);
            // }
            sensor.hum = humidity_get();

            ret = espnow_manager_send((uint8_t *)&sensor, sizeof(app_data_sensors_t));
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to send espnow");
            }
            timer_current = esp_timer_get_time();
        }
    }
}

void app_received_task(void *args)
{
    app_data_actuator_t actuator = {0};
    esp_err_t ret;
    for (;;)
    {
        if (xQueueReceive(app_queue, &actuator, pdMS_TO_TICKS(10)) == pdPASS)
        {
            // ESP_LOGI(TAG, "Received:\n perc_res=%d\n pwm_hum=%d", actuator.perc_res, actuator.pwm_hum);
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
            // alert_set(ALERT_FINISH);
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

    BaseType_t xRet = xTaskCreate(
        app_send_task,
        APP_TASK_SEND_NAME,
        APP_TASK_SEND_STACK_SIZE,
        NULL,
        APP_TASK_SEND_PRIOR,
        NULL);
    if (xRet != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to create task");
        return ESP_FAIL;
    }

    xRet = xTaskCreate(
        app_received_task,
        APP_TASK_RECEIVED_NAME,
        APP_TASK_RECEIVED_STACK_SIZE,
        NULL,
        APP_TASK_RECEIVED_PRIOR,
        NULL);
    if (xRet != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to create task");
        return ESP_FAIL;
    }

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