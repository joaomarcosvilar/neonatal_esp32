#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "comon.h"
#include "hardware/files/fs_manager.h"
#include "hardware/temperature/temperature.h"
#include "hardware/humidity/humidity.h"
#include "tasks/alert/alert.h"
#include "tasks/application/application.h"
#include "tasks/espnow_manager/espnow_manager.h"
#include "tasks/humidifier/humidifier.h"
#include "tasks/resistance/resistance.h"

#define TAG "MAIN"

void app_main()
{
	esp_err_t res = ESP_OK;

	// res = alert_init();
	// if (res != ESP_OK)
	// {
	// 	ESP_LOGE(TAG, "Failed to init alert");
	// 	return;
	// }

	res = fs_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init file sistem");
		alert_set(ALERT_INIT_FAIL);
		return;
	}

	res = temperature_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init tempeture");
		alert_set(ALERT_INIT_FAIL);
		return;
	}

	res = humidity_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init humidity");
		alert_set(ALERT_INIT_FAIL);
		return;
	}

	res = humidifier_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init humidifier");
		alert_set(ALERT_INIT_FAIL);
		return;
	}

	res = resistance_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init resistance");
		alert_set(ALERT_INIT_FAIL);
		return;
	}

	res = espnow_manager_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init espnow");
		alert_set(ALERT_INIT_FAIL);
		return;
	}

	res = application_init();
	if (res != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init aplication");
		alert_set(ALERT_INIT_FAIL);
		return;
	}
}