#ifndef COMON_H
#define COMON_H

#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"

#include "temperature/temperature.h"

typedef struct
{
    uint16_t perc_res;
    uint16_t pwm_hum;
} app_data_actuator_t;

typedef struct
{
    float temp[TEMPERATURE_MAX_SENSOR_COUNT];
    float hum;
} app_data_sensors_t;

#define TEST_NO_SENSORS     1

#endif