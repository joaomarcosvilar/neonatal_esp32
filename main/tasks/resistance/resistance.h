#ifndef RESISTANCE_H
#define RESISTANCE_H

#define RESISTANCE_ZC_GPIO              GPIO_NUM_15
#define RESISTANCE_TRIAC_GPIO           GPIO_NUM_16

#define RESISTANCE_DELAY_TRIAC_CLOSE           1000             //us
#define RESISTANCE_DELAY_SEMAPHORE_TIMEOUT      100

esp_err_t resistance_init(void);
esp_err_t resistance_set(uint16_t percent);

#endif