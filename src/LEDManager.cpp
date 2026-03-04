#include "LEDManager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "LED";

LEDManager::LEDManager() {}

void LEDManager::init() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_G) | (1ULL << LED_R) | (1ULL << LED_B),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    setColor(LED_OFF);
    ESP_LOGI(TAG, "LED initialized (G=%d, R=%d, B=%d)", LED_G, LED_R, LED_B);
}

void LEDManager::setColor(LEDColor color) {
    gpio_set_level(LED_G, color == LED_GREEN ? 1 : 0);
    gpio_set_level(LED_R, color == LED_RED ? 1 : 0);
    gpio_set_level(LED_B, color == LED_YELLOW ? 1 : 0);
}

void LEDManager::blink(LEDColor color, int times) {
    for (int i = 0; i < times; i++) {
        setColor(color);
        vTaskDelay(pdMS_TO_TICKS(200));
        setColor(LED_OFF);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
