#include "StateMachine.h"
#include "GameLogic.h"
#include "HandRecognition.h"
#include "AudioManager.h"
#include "LEDManager.h"
#include "esp_log.h"

static const char* TAG = "StateMachine";

// External module instances
extern AudioManager audio;
extern LEDManager led;
extern HandRecognition handRecog;
extern GameLogic gameLogic;

StateMachine::StateMachine(DisplayManager* disp, ButtonManager* btn)
    : currentState(STATE_IDLE), display(disp), buttons(btn),
      countdownValue(3), playerScore(0), pcScore(0), stateStartTime(0) {}

void StateMachine::reset() {
    currentState = STATE_IDLE;
    playerScore = 0;
    pcScore = 0;
    countdownValue = 3;
    display->showIdle();
    led.setColor(LED_OFF);
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
        audio.playSound(SOUND_COUNTDOWN);
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

    // Recognize player gesture
    RecognitionResult playerResult = handRecog.recognize();

    if (!playerResult.valid) {
        display->showRetry();
        audio.playSound(SOUND_COUNTDOWN);
        vTaskDelay(pdMS_TO_TICKS(1000));
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        ESP_LOGI(TAG, "Low confidence, retrying");
        return;
    }

    // Generate PC gesture
    Gesture pcGesture = gameLogic.generatePCGesture();

    // Determine winner
    GameResult result = gameLogic.determineWinner(playerResult.gesture, pcGesture);

    // Update score
    if (result == RESULT_PLAYER_WIN) {
        playerScore++;
    } else if (result == RESULT_PC_WIN) {
        pcScore++;
    }

    // Get display strings
    const char* playerEmoji = gameLogic.gestureToEmoji(playerResult.gesture);
    const char* pcEmoji = gameLogic.gestureToEmoji(pcGesture);
    const char* resultStr = gameLogic.resultToString(result);

    // Display result
    display->showResult(playerEmoji, pcEmoji, resultStr, playerScore, pcScore);

    // Play sound and set LED
    if (result == RESULT_PLAYER_WIN) {
        audio.playSound(SOUND_PLAYER_WIN);
        led.setColor(LED_GREEN);
    } else if (result == RESULT_PC_WIN) {
        audio.playSound(SOUND_PC_WIN);
        led.setColor(LED_RED);
    } else {
        audio.playSound(SOUND_DRAW);
        led.setColor(LED_YELLOW);
    }

    stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    currentState = STATE_RESULT;
    ESP_LOGI(TAG, "Result: %s, Score: %d:%d", resultStr, playerScore, pcScore);
}

void StateMachine::handleResult() {
    unsigned long elapsed = xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime;

    if (elapsed > 2000) {
        // Turn off LED before next round
        led.setColor(LED_OFF);

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
