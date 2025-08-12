#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "resistance\resistance.h"

#define TAG "RESISTANCE"
#define ESP_INTR_FLAG_DEFAULT 0
#define TIMER_NAME "resistance_timer_shoot"

static portMUX_TYPE resistance_spinlock = portMUX_INITIALIZER_UNLOCKED;
static esp_timer_handle_t resistance_timer;

volatile uint64_t last_zc_time_us = 0;
volatile uint64_t last_zc_period_us = 8.3 * 1000;

static uint16_t g_resistance_percent = 0;
static uint64_t resistance_timer_period = 0;
static bool intr_active = true;

void IRAM_ATTR resistance_pulse_triac(void)
{
    gpio_set_level(RESISTANCE_TRIAC_GPIO, 1);
    ets_delay_us(RESISTANCE_DELAY_TRIAC_CLOSE);
    gpio_set_level(RESISTANCE_TRIAC_GPIO, 0);
}

static void resistance_timer_cb(void *args)
{
    resistance_pulse_triac();
}

void IRAM_ATTR resistance_isr_handle(void *args)
{
    int64_t now = esp_timer_get_time();

    if (last_zc_time_us != 0)
    {
        last_zc_period_us = now - last_zc_time_us;
    }
    last_zc_time_us = now;

    uint64_t delay;

    portENTER_CRITICAL_ISR(&resistance_spinlock);
    delay = resistance_timer_period;
    portEXIT_CRITICAL_ISR(&resistance_spinlock);

    if (delay == 0)
    {
        resistance_pulse_triac();
    }
    else if (delay < last_zc_period_us)
    {
        esp_timer_start_once(resistance_timer, delay);
    }
}

esp_err_t resistance_set(uint16_t percent)
{
    if (percent > 100)
        percent = 100;

    portENTER_CRITICAL(&resistance_spinlock);

    g_resistance_percent = percent;

    if (percent == 100)
    {
        resistance_timer_period = 0;
        gpio_set_level(RESISTANCE_TRIAC_GPIO, 1);
        if (intr_active)
        {
            gpio_intr_disable(RESISTANCE_ZC_GPIO);
            intr_active = false;
        }
    }
    else if (percent == 0)
    {
        // resistance_timer_period = last_zc_period_us + 1000;
        gpio_set_level(RESISTANCE_TRIAC_GPIO, 0);
        if (intr_active)
        {
            gpio_intr_disable(RESISTANCE_ZC_GPIO);
            intr_active = false;
        }
    }
    else
    {
        if (!intr_active)
        {
            gpio_intr_enable(RESISTANCE_ZC_GPIO);
            intr_active = true;
        }

        resistance_timer_period = (last_zc_period_us * (100 - percent)) / 100;
    }

    portEXIT_CRITICAL(&resistance_spinlock);

    return ESP_OK;
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

    const esp_timer_create_args_t resistance_timer_args = {
        .callback = &resistance_timer_cb,
        .name = TIMER_NAME,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = true,
    };

    esp_err_t ret = esp_timer_create(&resistance_timer_args, &resistance_timer);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create timer");
        return ret;
    }
    gpio_intr_enable(RESISTANCE_ZC_GPIO);

    resistance_set(0);

    return ESP_OK;
}
