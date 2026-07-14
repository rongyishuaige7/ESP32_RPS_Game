#include "ButtonManager.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"
#include <Arduino.h>

static const char* TAG = "Button";

ButtonManager::ButtonManager()
    : startPressed(false), resetPressed(false),
      lastStartPressTime(0), lastResetPressTime(0) {}

void IRAM_ATTR ButtonManager::gpio_isr_handler(void* arg) {
    volatile bool* pressed = (volatile bool*)arg;
    *pressed = true;
}

void ButtonManager::init() {
    // 配置上拉电阻
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << START_BTN) | (1ULL << RESET_BTN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);

    // 安装中断服务
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(START_BTN, gpio_isr_handler, (void*)&startPressed);
    gpio_isr_handler_add(RESET_BTN, gpio_isr_handler, (void*)&resetPressed);

    ESP_LOGI(TAG, "Buttons initialized");
}

bool ButtonManager::isStartPressed() {
    unsigned long now = millis();
    if (startPressed && (now - lastStartPressTime) >= DEBOUNCE_MS) {
        startPressed = false;
        lastStartPressTime = now;
        return true;
    }
    if (startPressed) startPressed = false;  // 消抖窗口内忽略
    return false;
}

bool ButtonManager::isResetPressed() {
    unsigned long now = millis();
    if (resetPressed && (now - lastResetPressTime) >= DEBOUNCE_MS) {
        resetPressed = false;
        lastResetPressTime = now;
        return true;
    }
    if (resetPressed) resetPressed = false;
    return false;
}
