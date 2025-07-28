#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "hardware/files/fs_manager.h"
#include "hardware/temperature/temperature.h"
#include "hardware/humidity/humidity.h"
#include "tasks/humidifier/humidifier.h"
#include "tasks/resistance/resistance.h"

#define TAG "MAIN"

void app_main()
{
	esp_err_t res = ESP_OK;

	res = fs_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init file sistem");
		return;
	}

	res = temperature_init();
	{
		ESP_LOGE(TAG, "Failed to init tempeture");
		return;
	}

	res = humidity_init();
	{
		ESP_LOGE(TAG, "Failed to init humidity");
		return;
	}

	res = humidifier_init();
	{
		ESP_LOGE(TAG, "Failed to init humidifier");
		return;
	}

	res = resistance_init();
	{
		ESP_LOGE(TAG, "Failed to init resistance");
		return;
	}
}