#ifndef ALERT_H
#define ALERT_H

typedef enum{
    ALERT_CLEAN = 0,
    ALERT_ESPNOW_SEND_FAIL,
    ALERT_INIT_FAIL,
    ALERT_FINISH,
    ALERT_KEEP_ALIVE
}alert_color_e;

#define ALERT_LED_GPIO GPIO_NUM_48

esp_err_t alert_init(void);
esp_err_t alert_set(alert_color_e alert);

#endif