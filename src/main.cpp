#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "DisplayManager.h"
#include "ButtonManager.h"
#include "StateMachine.h"

static const char* TAG = "RPS_Game";

DisplayManager display;
ButtonManager buttons;
StateMachine stateMachine(&display, &buttons);

extern "C" void app_main() {
    ESP_LOGI(TAG, "RPS Game Starting...");

    display.init();
    buttons.init();
    stateMachine.reset();

    while(true) {
        stateMachine.update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
