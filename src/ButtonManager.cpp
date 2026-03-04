#include "ButtonManager.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"

static const char* TAG = "Button";

ButtonManager::ButtonManager() : startPressed(false), resetPressed(false) {}

void IRAM_ATTR ButtonManager::gpio_isr_handler(void* arg) {
    volatile bool* pressed = (volatile bool*)arg;
    *pressed = true;
}

void ButtonManager::init() {
    // Configure pull-up resistors
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << START_BTN) | (1ULL << RESET_BTN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);

    // Install interrupt service
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(START_BTN, gpio_isr_handler, (void*)&startPressed);
    gpio_isr_handler_add(RESET_BTN, gpio_isr_handler, (void*)&resetPressed);

    ESP_LOGI(TAG, "Buttons initialized");
}

bool ButtonManager::isStartPressed() {
    if (startPressed) {
        startPressed = false;
        return true;
    }
    return false;
}

bool ButtonManager::isResetPressed() {
    if (resetPressed) {
        resetPressed = false;
        return true;
    }
    return false;
}
