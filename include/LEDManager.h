#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "driver/gpio.h"

enum LEDColor {
    LED_OFF,
    LED_GREEN,   // Player win
    LED_RED,     // PC win
    LED_YELLOW   // Draw
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
