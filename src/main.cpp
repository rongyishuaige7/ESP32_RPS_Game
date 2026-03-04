#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "DisplayManager.h"
#include "ButtonManager.h"
#include "StateMachine.h"
#include "GameLogic.h"
#include "HandRecognition.h"
#include "AudioManager.h"
#include "LEDManager.h"

static const char* TAG = "RPS_Game";

// Global module instances
DisplayManager display;
ButtonManager buttons;
GameLogic gameLogic;
HandRecognition handRecog;
AudioManager audio;
LEDManager led;
StateMachine stateMachine(&display, &buttons);

extern "C" void app_main() {
    ESP_LOGI(TAG, "RPS Game Starting...");

    // Initialize all modules
    display.init();
    buttons.init();
    audio.init();
    led.init();
    handRecog.init();

    // Start game in IDLE state
    stateMachine.reset();

    while(true) {
        stateMachine.update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
