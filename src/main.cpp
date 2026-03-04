#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "RPS_Game";

enum GameState {
    STATE_IDLE,
    STATE_COUNTDOWN,
    STATE_PLAYING,
    STATE_RESULT
};

GameState currentState = STATE_IDLE;

extern "C" void app_main() {
    ESP_LOGI(TAG, "RPS Game Starting...");

    while(true) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
