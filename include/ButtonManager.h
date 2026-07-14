#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class ButtonManager {
public:
    ButtonManager();
    void init();
    bool isStartPressed();
    bool isResetPressed();

private:
    static const gpio_num_t START_BTN = GPIO_NUM_0;
    static const gpio_num_t RESET_BTN = GPIO_NUM_47;
    static const unsigned long DEBOUNCE_MS = 50;
    volatile bool startPressed;
    volatile bool resetPressed;
    unsigned long lastStartPressTime;
    unsigned long lastResetPressTime;
    static void IRAM_ATTR gpio_isr_handler(void* arg);
};

#endif
