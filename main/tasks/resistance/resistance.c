#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "rom/ets_sys.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "resistance\resistance.h"

#define RESISTANCE_TASK_NAME "resistance"
#define RESISTANCE_TASK_STACK_SIZE 1024 * 1
#define RESISTANCE_TASK_PRIOR 2

#define ESP_INTR_FLAG_DEFAULT 0

#define TAG "RESISTANCE"

static TaskHandle_t resistance_task_handle;
static SemaphoreHandle_t resistance_semaphore;
static uint16_t g_resistance_percent = 0;

volatile int64_t g_resistance_last_zc_time = 0;
volatile int64_t g_resistance_last_semicycle_us = 0;

void IRAM_ATTR resistance_isr_handle(void *args)
{
    int64_t now = esp_timer_get_time();
    g_resistance_last_semicycle_us = now - g_resistance_last_zc_time;
    g_resistance_last_zc_time = now;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(resistance_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void resistance_pulse_triac(void)
{
    gpio_set_level(RESISTANCE_TRIAC_GPIO, 1);
    ets_delay_us(RESISTANCE_DELAY_TRIAC_CLOSE);
    gpio_set_level(RESISTANCE_TRIAC_GPIO, 0);
}

void resistance_task(void *args)
{
    uint32_t delay = 0;
    uint16_t percent = 0;

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (xSemaphoreTake(resistance_semaphore, pdMS_TO_TICKS(2)) == pdTRUE)
        {
            percent = g_resistance_percent;
            xSemaphoreGive(resistance_semaphore);

            delay = (uint32_t)((100 - percent) * g_resistance_last_semicycle_us) / 100; // us

            if (delay)
            {
                ets_delay_us(delay);
                resistance_pulse_triac();
            }
        }
    }
}

esp_err_t resistance_init(void)
{
    gpio_config_t zc_conf = {};
    zc_conf.intr_type = GPIO_INTR_POSEDGE;

    zc_conf.pin_bit_mask = 1ULL << RESISTANCE_ZC_GPIO;
    zc_conf.mode = GPIO_MODE_INPUT;
    zc_conf.pull_up_en = 1;
    gpio_config(&zc_conf);
    gpio_set_intr_type(RESISTANCE_ZC_GPIO, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(RESISTANCE_ZC_GPIO, resistance_isr_handle, NULL);

    gpio_config_t triac_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 1ULL << RESISTANCE_TRIAC_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    gpio_config(&triac_conf);
    gpio_set_level(RESISTANCE_TRIAC_GPIO, 0);

    resistance_semaphore = xSemaphoreCreateMutex();
    if (resistance_semaphore == NULL)
    {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return ESP_FAIL;
    }

    BaseType_t xRet = xTaskCreate(resistance_task, RESISTANCE_TASK_NAME, RESISTANCE_TASK_STACK_SIZE, NULL, RESISTANCE_TASK_PRIOR, &resistance_task_handle);
    if (xRet != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to create task (E: %d)", xRet);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t resistance_set(uint16_t percent)
{
    if (percent > 100)
        percent = 100;

    if (xSemaphoreTake(resistance_semaphore, pdMS_TO_TICKS(RESISTANCE_DELAY_SEMAPHORE_TIMEOUT)) == pdTRUE)
    {
        g_resistance_percent = percent;
        xSemaphoreGive(resistance_semaphore);
        return ESP_OK;
    }

    return ESP_FAIL;
}
