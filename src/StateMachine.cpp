#include "StateMachine.h"
#include "GameLogic.h"
#include "HandRecognition.h"
#include "AudioManager.h"
#include "LEDManager.h"
#include "esp_log.h"

static const char* TAG = "StateMachine";

StateMachine::StateMachine(DisplayManager* disp, ButtonManager* btn,
                         AudioManager* aud, LEDManager* ledPtr,
                         HandRecognition* handRecogPtr, GameLogic* gameLogicPtr)
    : currentState(STATE_IDLE), display(disp), buttons(btn),
      audio(aud), led(ledPtr), handRecog(handRecogPtr), gameLogic(gameLogicPtr),
      countdownValue(3), playerScore(0), pcScore(0), stateStartTime(0) {}

void StateMachine::reset() {
    currentState = STATE_IDLE;
    playerScore = 0;
    pcScore = 0;
    countdownValue = 3;
    display->setDirty(true);
    display->showIdle();
    display->setDirty(false);
    led->setColor(LED_OFF);
    ESP_LOGI(TAG, "Game reset to IDLE state");
}

void StateMachine::update() {
    // 任意时刻处理复位键
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
    if (display->isDirty()) {
        display->showIdle();
        display->setDirty(false);
    }
    if (buttons->isStartPressed()) {
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        display->showCountdown(3);
        audio->playSound(SOUND_COUNTDOWN);
        ESP_LOGI(TAG, "Starting countdown");
    }
}

void StateMachine::handleCountdown() {
    unsigned long elapsed = xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime;
    int newCountdown = 3 - (elapsed / 1000);

    if (newCountdown != countdownValue && newCountdown >= 0) {
        countdownValue = newCountdown;
        display->showCountdown(countdownValue);
        if (countdownValue > 0) {
            audio->playSound(SOUND_COUNTDOWN);
        } else {
            audio->playSound(SOUND_GO);
        }
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

    // 识别玩家手势
    RecognitionResult playerResult = handRecog->recognize();

    if (!playerResult.valid) {
        display->showRetry();
        audio->playSound(SOUND_COUNTDOWN);
        vTaskDelay(pdMS_TO_TICKS(1000));
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        ESP_LOGI(TAG, "Low confidence, retrying");
        return;
    }

    // 生成电脑出拳
    Gesture pcGesture = gameLogic->generatePCGesture();

    // 先显示电脑出拳 1 秒，再显示完整结果
    display->showPCGesture(gameLogic->gestureToName(pcGesture));
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 判定胜负
    GameResult result = gameLogic->determineWinner(playerResult.gesture, pcGesture);

    // 更新比分
    if (result == RESULT_PLAYER_WIN) {
        playerScore++;
    } else if (result == RESULT_PC_WIN) {
        pcScore++;
    }

    // 获取显示用字符串
    const char* playerEmoji = gameLogic->gestureToEmoji(playerResult.gesture);
    const char* pcEmoji = gameLogic->gestureToEmoji(pcGesture);
    const char* resultStr = gameLogic->resultToString(result);

    // 显示结果
    display->showResult(playerEmoji, pcEmoji, resultStr, playerScore, pcScore);

    // 播放音效并设置 LED
    if (result == RESULT_PLAYER_WIN) {
        audio->playSound(SOUND_PLAYER_WIN);
        led->setColor(LED_GREEN);
    } else if (result == RESULT_PC_WIN) {
        audio->playSound(SOUND_PC_WIN);
        led->setColor(LED_RED);
    } else {
        audio->playSound(SOUND_DRAW);
        led->setColor(LED_YELLOW);
    }

    stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    currentState = STATE_RESULT;
    ESP_LOGI(TAG, "Result: %s, Score: %d:%d", resultStr, playerScore, pcScore);
}

void StateMachine::handleResult() {
    unsigned long elapsed = xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime;

    if (elapsed > 2000) {
        led->setColor(LED_OFF);
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        display->showCountdown(3);
        audio->playSound(SOUND_COUNTDOWN);
        ESP_LOGI(TAG, "Auto starting next round");
    }
}

bool StateMachine::isTimeElapsed(unsigned long ms) {
    return (xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime) > ms;
}
