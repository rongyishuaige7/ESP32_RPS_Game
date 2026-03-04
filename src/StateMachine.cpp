#include "StateMachine.h"
#include "esp_log.h"

static const char* TAG = "StateMachine";

StateMachine::StateMachine(DisplayManager* disp, ButtonManager* btn)
    : currentState(STATE_IDLE), display(disp), buttons(btn),
      countdownValue(3), playerScore(0), pcScore(0), stateStartTime(0) {}

void StateMachine::reset() {
    currentState = STATE_IDLE;
    playerScore = 0;
    pcScore = 0;
    countdownValue = 3;
    display->showIdle();
    ESP_LOGI(TAG, "Game reset to IDLE state");
}

void StateMachine::update() {
    // Handle reset button anytime
    if (buttons->isResetPressed()) {
        reset();
        return;
    }

    switch(currentState) {
        case STATE_IDLE: handleIdle(); break;
        case STATE_COUNTDOWN: handleCountdown(); break;
        case STATE_PLAYING: handlePlaying(); break;
        case STATE_RESULT: handleResult(); break;
    }
}

void StateMachine::handleIdle() {
    display->showIdle();
    if (buttons->isStartPressed()) {
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        ESP_LOGI(TAG, "Starting countdown");
    }
}

void StateMachine::handleCountdown() {
    unsigned long elapsed = xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime;
    int newCountdown = 3 - (elapsed / 1000);

    if (newCountdown != countdownValue && newCountdown >= 0) {
        countdownValue = newCountdown;
        display->showCountdown(countdownValue);
        ESP_LOGI(TAG, "Countdown: %d", countdownValue);
    }

    if (countdownValue <= 0) {
        currentState = STATE_PLAYING;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        ESP_LOGI(TAG, "Entering PLAYING state");
    }
}

void StateMachine::handlePlaying() {
    display->showPlaying();
    ESP_LOGI(TAG, "Playing - gesture recognition");

    // TODO: Call gesture recognition
    // For now, skip to result
    currentState = STATE_RESULT;
    stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void StateMachine::handleResult() {
    // TODO: Display result with sound and LED
    unsigned long elapsed = xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime;

    if (elapsed > 2000) {
        // Auto start next round
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        ESP_LOGI(TAG, "Auto starting next round");
    }
}

bool StateMachine::isTimeElapsed(unsigned long ms) {
    return (xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime) > ms;
}
