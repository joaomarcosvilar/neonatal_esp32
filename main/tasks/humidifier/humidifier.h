#ifndef HUMDIFIER_H
#define HUMDIFIER_H

#define HUMIDFIER_GPIO          GPIO_NUM_10

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          HUMIDFIER_GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (1000) // Frequency in Hertz. Set frequency at 4 kHz


esp_err_t humidifier_init(void);
esp_err_t humidifier_set(uint16_t percent);

#endif