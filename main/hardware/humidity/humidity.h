#ifndef HUMIDITY_H
#define HUMIDITY_H

#define HUMIDITY_SENSOR_ADDR AHT_I2C_ADDRESS_GND
#define HUMIDITY_I2C_PORT I2C_NUM_0

#define HUMIDITY_I2C_SDA_GPIO GPIO_NUM_21
#define HUMIDITY_I2C_SCL_GPIO GPIO_NUM_22

esp_err_t humidity_init(void);
float humidity_get(void);

#endif