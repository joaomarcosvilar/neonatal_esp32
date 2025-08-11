#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "comon.h"
#include "hardware/files/fs_manager.h"
#include "hardware/temperature/temperature.h"
#include "hardware/humidity/humidity.h"
#include "tasks/application/application.h"
#include "tasks/espnow_manager/espnow_manager.h"
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
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init tempeture");
		return;
	}

	ESP_LOGI(TAG, "Temperature: %.2f", temperature_get(1));

	res = humidity_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init humidity");
		return;
	}
	ESP_LOGI(TAG, "Humidity: %.2f", humidity_get());


	res = humidifier_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init humidifier");
		return;
	}

	res = resistance_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init resistance");
		return;
	}

	res = espnow_manager_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init espnow");
		return;
	}

	res = application_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init aplication");
		return;
	}
}