#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "driver/gpio.h"

enum LEDColor {
    LED_OFF,
    LED_GREEN,   // 玩家赢
    LED_RED,     // 电脑赢
    LED_YELLOW   // 平局
};

class LEDManager {
public:
    LEDManager();
    void init();
    void setColor(LEDColor color);
    void blink(LEDColor color, int times);

private:
    static const gpio_num_t LED_G = GPIO_NUM_45;
    static const gpio_num_t LED_R = GPIO_NUM_48;
    static const gpio_num_t LED_B = GPIO_NUM_38;
};

#endif
