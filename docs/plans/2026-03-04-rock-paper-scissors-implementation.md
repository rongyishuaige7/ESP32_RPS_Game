# ESP32 剪刀石头布游戏 - 实施计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 基于 ESP32-S3-CAM 实现剪刀石头布游戏，包含手势识别、OLED显示、声光反馈

**Architecture:** 有限状态机管理游戏流程，模块化设计（状态机、识别、游戏逻辑、显示、音频、LED）

**Tech Stack:** ESP-IDF v5.x + PlatformIO, U8g2 (OLED), esp32-camera

---

## 硬件接线参考

```
ESP32-S3-CAM 引脚分配:
- OLED SDA: GPIO 4
- OLED SCL: GPIO 15
- 按键开始: GPIO 0
- 按键复位: GPIO 47
- 蜂鸣器: GPIO 21
- LED 红: GPIO 48
- LED 绿: GPIO 45
- LED 蓝: GPIO 38 (原计划复用改为独立)
```

---

## Task 1: 项目初始化与 PlatformIO 配置

**Files:**
- Create: `platformio.ini`
- Create: `src/main.cpp`
- Create: `include/README`

**Step 1: 创建项目结构**

```bash
cd /home/rongyi/桌面/ESP32_RPS_Game
mkdir -p src include lib test
```

**Step 2: 创建 platformio.ini**

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32@6.4.0
board = esp32-s3-devkitc-1
framework = espidf
monitor_speed = 115200
upload_speed = 921600

build_flags =
    -DCAMERA_MODEL_ESP32S3_EYE
    -DBOARD_HAS_PSRAM
    -I include

lib_deps =
    olikraus/U8g2@^2.34
```

**Step 3: 创建基础 main.cpp (空状态机框架)**

```cpp
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
```

**Step 4: 验证编译**

Run: `pio run -e esp32-s3-devkitc-1`
Expected: 编译成功（会有一些警告但无错误）

**Step 5: Commit**

```bash
git init
git add platformio.ini src/main.cpp include/README
git commit -m "feat: 初始化 PlatformIO 项目结构"
```

---

## Task 2: OLED 显示模块 (DisplayManager)

**Files:**
- Create: `src/DisplayManager.cpp`
- Create: `include/DisplayManager.h`
- Modify: `src/main.cpp`

**Step 1: 创建 DisplayManager.h**

```cpp
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <U8g2lib.h>

class DisplayManager {
public:
    DisplayManager();
    void init();
    void showIdle();
    void showCountdown(int seconds);
    void showPlaying();
    void showResult(const char* playerGesture, const char* pcGesture,
                    const char* result, int playerScore, int pcScore);
    void clear();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2;
    void drawText(const char* line1, const char* line2 = nullptr);
};

#endif
```

**Step 2: 创建 DisplayManager.cpp**

```cpp
#include "DisplayManager.h"
#include <stdio.h>

// 表情符号映射
const char* GESTURE_STONE = "\u270A";  // ✊
const char* GESTURE_SCISSORS = "\u270C"; // ✌️
const char* GESTURE_PAPER = "\u270B";   // ✋

DisplayManager::DisplayManager() {
    static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 4, 15);
    this->u8g2 = &u8g2;
}

void DisplayManager::init() {
    u8g2->begin();
    u8g2->setFlipMode(1);
    u8g2->setFont(u8g2_font_ncenB14_tr);
}

void DisplayManager::showIdle() {
    u8g2->clearBuffer();
    drawText("Waiting...", "Press START");
    u8g2->sendBuffer();
}

void DisplayManager::showCountdown(int seconds) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Countdown: %d", seconds);
    u8g2->clearBuffer();
    drawText(buf, "Get Ready!");
    u8g2->sendBuffer();
}

void DisplayManager::showPlaying() {
    u8g2->clearBuffer();
    drawText("Recognizing...", "Show gesture!");
    u8g2->sendBuffer();
}

void DisplayManager::showResult(const char* playerGesture, const char* pcGesture,
                                const char* result, int playerScore, int pcScore) {
    char line1[32];
    char line2[32];
    snprintf(line1, sizeof(line1), "You:%s PC:%s", playerGesture, pcGesture);
    snprintf(line2, sizeof(line2), "%s (%d:%d)", result, playerScore, pcScore);

    u8g2->clearBuffer();
    drawText(line1, line2);
    u8g2->sendBuffer();
}

void DisplayManager::clear() {
    u8g2->clearBuffer();
    u8g2->sendBuffer();
}

void DisplayManager::drawText(const char* line1, const char* line2) {
    u8g2->setCursor(0, 20);
    u8g2->print(line1);
    if (line2) {
        u8g2->setCursor(0, 45);
        u8g2->print(line2);
    }
}
```

**Step 3: 修改 main.cpp 添加 OLED 测试代码**

在 `app_main()` 中添加:
```cpp
#include "DisplayManager.h"
DisplayManager display;

extern "C" void app_main() {
    ESP_LOGI(TAG, "RPS Game Starting...");

    display.init();
    display.showIdle();

    while(true) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

**Step 4: 编译上传测试**

Run: `pio run -t upload -e esp32-s3-devkitc-1`
Expected: OLED 显示"Waiting... Press START"

**Step 5: Commit**

```bash
git add src/DisplayManager.cpp include/DisplayManager.h src/main.cpp
git commit -m "feat: 添加 OLED 显示模块"
```

---

## Task 3: 按键输入模块

**Files:**
- Create: `src/ButtonManager.cpp`
- Create: `include/ButtonManager.h`
- Modify: `src/main.cpp`

**Step 1: 创建 ButtonManager.h**

```cpp
#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <gpio/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class ButtonManager {
public:
    ButtonManager();
    void init();
    bool isStartPressed();
    bool isResetPressed();
    void IRAM_ATTR gpio_isr_handler(void* arg);

private:
    static const gpio_num_t START_BTN = GPIO_NUM_0;
    static const gpio_num_t RESET_BTN = GPIO_NUM_47;
    volatile bool startPressed;
    volatile bool resetPressed;
};

#endif
```

**Step 2: 创建 ButtonManager.cpp**

```cpp
#include "ButtonManager.h"
#include <esp_log.h>

static const char* TAG = "Button";

ButtonManager::ButtonManager() : startPressed(false), resetPressed(false) {}

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
    gpio_install_isr_service(0);
    gpio_isr_handler_add(START_BTN, gpio_isr_handler, (void*)&startPressed);
    gpio_isr_handler_add(RESET_BTN, gpio_isr_handler, (void*)&resetPressed);

    ESP_LOGI(TAG, "Buttons initialized");
}

void IRAM_ATTR ButtonManager::gpio_isr_handler(void* arg) {
    volatile bool* pressed = (volatile bool*)arg;
    *pressed = true;
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
```

**Step 3: 修改 main.cpp 添加按键测试**

```cpp
#include "ButtonManager.h"
ButtonManager buttons;

extern "C" void app_main() {
    display.init();
    buttons.init();
    display.showIdle();

    while(true) {
        if (buttons.isStartPressed()) {
            ESP_LOGI(TAG, "START pressed!");
        }
        if (buttons.isResetPressed()) {
            ESP_LOGI(TAG, "RESET pressed!");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

**Step 4: 编译上传测试**

**Step 5: Commit**

```bash
git add src/ButtonManager.cpp include/ButtonManager.h src/main.cpp
git commit -m "feat: 添加按键输入模块"
```

---

## Task 4: 有限状态机实现

**Files:**
- Create: `src/StateMachine.cpp`
- Create: `include/StateMachine.h`
- Modify: `src/main.cpp`

**Step 1: 创建 StateMachine.h**

```cpp
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "DisplayManager.h"
#include "ButtonManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

enum GameState {
    STATE_IDLE,
    STATE_COUNTDOWN,
    STATE_PLAYING,
    STATE_RESULT
};

class StateMachine {
public:
    StateMachine(DisplayManager* display, ButtonManager* buttons);
    void update();
    void reset();

private:
    GameState currentState;
    DisplayManager* display;
    ButtonManager* buttons;
    int countdownValue;
    int playerScore;
    int pcScore;
    unsigned long stateStartTime;

    void handleIdle();
    void handleCountdown();
    void handlePlaying();
    void handleResult();
    bool isTimeElapsed(unsigned long ms);
};

#endif
```

**Step 2: 创建 StateMachine.cpp**

```cpp
#include "StateMachine.h"
#include <esp_log.h>

static const char* TAG = "StateMachine";

StateMachine::StateMachine(DisplayManager* disp, ButtonManager* btn)
    : currentState(STATE_IDLE), display(disp), buttons(btn),
      countdownValue(3), playerScore(0), pcScore(0) {}

void StateMachine::reset() {
    currentState = STATE_IDLE;
    playerScore = 0;
    pcScore = 0;
    display->showIdle();
}

void StateMachine::update() {
    switch(currentState) {
        case STATE_IDLE: handleIdle(); break;
        case STATE_COUNTDOWN: handleCountdown(); break;
        case STATE_PLAYING: handlePlaying(); break;
        case STATE_RESULT: handleResult(); break;
    }

    if (buttons->isResetPressed()) {
        reset();
    }
}

void StateMachine::handleIdle() {
    display->showIdle();
    if (buttons->isStartPressed()) {
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
}

void StateMachine::handleCountdown() {
    unsigned long elapsed = xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime;
    int newCountdown = 3 - (elapsed / 1000);

    if (newCountdown != countdownValue && newCountdown >= 0) {
        countdownValue = newCountdown;
        display->showCountdown(countdownValue);
    }

    if (countdownValue <= 0) {
        currentState = STATE_PLAYING;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
}

void StateMachine::handlePlaying() {
    display->showPlaying();
    // TODO: 调用手势识别
    currentState = STATE_RESULT;
}

void StateMachine::handleResult() {
    // TODO: 显示结果
    unsigned long elapsed = xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime;
    if (elapsed > 2000) {
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
}

bool StateMachine::isTimeElapsed(unsigned long ms) {
    return (xTaskGetTickCount() * portTICK_PERIOD_MS - stateStartTime) > ms;
}
```

**Step 3: 修改 main.cpp 集成状态机**

```cpp
StateMachine stateMachine(&display, &buttons);

extern "C" void app_main() {
    display.init();
    buttons.init();
    stateMachine.reset();

    while(true) {
        stateMachine.update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

**Step 4: 编译测试**

**Step 5: Commit**

```bash
git add src/StateMachine.cpp include/StateMachine.h src/main.cpp
git commit -m "feat: 添加有限状态机"
```

---

## Task 5: 游戏逻辑模块 (GameLogic)

**Files:**
- Create: `src/GameLogic.cpp`
- Create: `include/GameLogic.h`

**Step 1: 创建 GameLogic.h**

```cpp
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>

enum Gesture {
    GESTURE_NONE = 0,
    GESTURE_STONE = 1,      // 石头 ✊
    GESTURE_SCISSORS = 2,  // 剪刀 ✌️
    GESTURE_PAPER = 3      // 布 ✋
};

enum GameResult {
    RESULT_PLAYER_WIN,
    RESULT_PC_WIN,
    RESULT_DRAW
};

class GameLogic {
public:
    GameLogic();
    Gesture generatePCGesture();
    GameResult determineWinner(Gesture player, Gesture pc);
    const char* gestureToEmoji(Gesture g);
    const char* resultToString(GameResult r);

private:
};

#endif
```

**Step 2: 创建 GameLogic.cpp**

```cpp
#include "GameLogic.h"
#include <stdlib.h>

GameLogic::GameLogic() {}

Gesture GameLogic::generatePCGesture() {
    return (Gesture)((esp_random() % 3) + 1);
}

GameResult GameLogic::determineWinner(Gesture player, Gesture pc) {
    if (player == pc) return RESULT_DRAW;

    if ((player == GESTURE_STONE && pc == GESTURE_SCISSORS) ||
        (player == GESTURE_SCISSORS && pc == GESTURE_PAPER) ||
        (player == GESTURE_PAPER && pc == GESTURE_STONE)) {
        return RESULT_PLAYER_WIN;
    }

    return RESULT_PC_WIN;
}

const char* GameLogic::gestureToEmoji(Gesture g) {
    switch(g) {
        case GESTURE_STONE: return "\u270A";     // ✊
        case GESTURE_SCISSORS: return "\u270C"; // ✌️
        case GESTURE_PAPER: return "\u270B";    // ✋
        default: return "?";
    }
}

const char* GameLogic::resultToString(GameResult r) {
    switch(r) {
        case RESULT_PLAYER_WIN: return "You Win!";
        case RESULT_PC_WIN: return "PC Wins!";
        case RESULT_DRAW: return "Draw!";
        default: return "?";
    }
}
```

**Step 3: 编译测试**

**Step 4: Commit**

---

## Task 6: 蜂鸣器音频模块

**Files:**
- Create: `src/AudioManager.cpp`
- Create: `include/AudioManager.h`

**Step 1: 创建 AudioManager.h**

```cpp
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <driver/ledc.h>

enum SoundType {
    SOUND_COUNTDOWN,
    SOUND_PLAYER_WIN,
    SOUND_PC_WIN,
    SOUND_DRAW
};

class AudioManager {
public:
    AudioManager();
    void init();
    void playSound(SoundType type);
    void stop();

private:
    static const gpio_num_t BUZZER_PIN = GPIO_NUM_21;
    void beep(uint32_t freq, uint32_t duration_ms);
    void playSequence(const uint32_t* freqs, const uint32_t* durations, int count);
};

#endif
```

**Step 2: 创建 AudioManager.cpp**

```cpp
#include "AudioManager.h"
#include <esp_log.h>

static const char* TAG = "Audio";

AudioManager::AudioManager() {}

void AudioManager::init() {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel);
    ledc_fade_func_install(0);

    ESP_LOGI(TAG, "Buzzer initialized");
}

void AudioManager::playSound(SoundType type) {
    switch(type) {
        case SOUND_COUNTDOWN:
            beep(1000, 100);
            break;
        case SOUND_PLAYER_WIN:
            // 升调 Do-Re-Do
            beep(523, 150);
            vTaskDelay(pdMS_TO_TICKS(100));
            beep(587, 150);
            vTaskDelay(pdMS_TO_TICKS(100));
            beep(523, 200);
            break;
        case SOUND_PC_WIN:
            // 降调
            beep(587, 150);
            vTaskDelay(pdMS_TO_TICKS(100));
            beep(523, 250);
            break;
        case SOUND_DRAW:
            beep(523, 100);
            vTaskDelay(pdMS_TO_TICKS(50));
            beep(659, 150);
            break;
    }
}

void AudioManager::beep(uint32_t freq, uint32_t duration_ms) {
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void AudioManager::stop() {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
```

**Step 3: 编译测试**

**Step 4: Commit**

---

## Task 7: LED 模块

**Files:**
- Create: `src/LEDManager.cpp`
- Create: `include/LEDManager.h`

**Step 1: 创建 LEDManager.h**

```cpp
#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <driver/gpio.h>

enum LEDColor {
    LED_OFF,
    LED_GREEN,   // 玩家赢
    LED_RED,     // 电脑赢
    LED_YELLOW   // 平局
};

class LEDManager {
public:
    LEDManager();
    void init();
    void setColor(LEDColor color);
    void blink(LEDColor color, int times);

private:
    static const gpio_num_t LED_G = GPIO_NUM_45;
    static const gpio_num_t LED_R = GPIO_NUM_48;
    static const gpio_num_t LED_B = GPIO_NUM_38;
};

#endif
```

**Step 2: 创建 LEDManager.cpp**

```cpp
#include "LEDManager.h"
#include <esp_log.h>

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
    ESP_LOGI(TAG, "LED initialized");
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
```

**Step 3: 编译测试**

**Step 4: Commit**

---

## Task 8: 手势识别模块 (基础版 - 肤色分割)

**Files:**
- Create: `src/HandRecognition.cpp`
- Create: `include/HandRecognition.h`

**Step 1: 创建 HandRecognition.h**

```cpp
#ifndef HAND_RECOGNITION_H
#define HAND_RECOGNITION_H

#include <esp_camera.h>
#include "GameLogic.h"

struct RecognitionResult {
    Gesture gesture;
    float confidence;
    bool valid;
};

class HandRecognition {
public:
    HandRecognition();
    void init();
    RecognitionResult recognize();

private:
    bool detectSkinColor(uint8_t r, uint8_t g, uint8_t b);
    int countFingers(uint8_t* img, int width, int height);
    Gesture fingersToGesture(int fingerCount);
};

#endif
```

**Step 2: 创建 HandRecognition.cpp (简化版)**

```cpp
#include "HandRecognition.h"
#include <esp_log.h>
#include <esp_random.h>

static const char* TAG = "HandRecog";

HandRecognition::HandRecognition() {}

void HandRecognition::init() {
    camera_config_t config = {
        .pin_pwdn = -1,
        .pin_reset = -1,
        .pin_xclk = 15,
        .pin_sscb_sda = 4,
        .pin_sscb_scl = 15, // 复用
        .pin_d7 = 16,
        .pin_d6 = 17,
        .pin_d5 = 18,
        .pin_d4 = 12,
        .pin_d3 = 10,
        .pin_d2 = 8,
        .pin_d1 = 9,
        .pin_d0 = 11,
        .pin_vsync = 6,
        .pin_href = 7,
        .pin_pclk = 13,
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_QVGA,
        .jpeg_quality = 12,
        .fb_count = 2
    };

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(err));
    }
}

RecognitionResult HandRecognition::recognize() {
    RecognitionResult result = {GESTURE_NONE, 0.0f, false};

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return result;
    }

    // 简化版：随机返回手势用于测试
    // 实际需要实现 YCrCb 肤色分割 + 手指计数
    result.gesture = (Gesture)((esp_random() % 3) + 1);
    result.confidence = 0.85f;
    result.valid = (result.confidence >= 0.75f);

    esp_camera_fb_return(fb);
    return result;
}

bool HandRecognition::detectSkinColor(uint8_t r, uint8_t g, uint8_t b) {
    // YCrCb 肤色检测简化版
    return (r > 95 && g > 40 && b > 20 &&
            r > g && r > b &&
            abs(r - g) > 15 && r - b > 15);
}

int HandRecognition::countFingers(uint8_t* img, int width, int height) {
    // TODO: 实现手指计数算法
    return 0;
}

Gesture HandRecognition::fingersToGesture(int fingerCount) {
    if (fingerCount <= 1) return GESTURE_STONE;
    if (fingerCount == 2) return GESTURE_SCISSORS;
    return GESTURE_PAPER;
}
```

**Step 3: 编译测试**

**Step 4: Commit**

---

## Task 9: 完整集成与测试

**Files:**
- Modify: `src/main.cpp`
- Modify: `src/StateMachine.cpp`

**Step 1: 更新 StateMachine.cpp 集成所有模块**

在 handlePlaying() 中添加完整游戏逻辑:
```cpp
void StateMachine::handlePlaying() {
    display->showPlaying();

    // 识别玩家手势
    RecognitionResult playerResult = handRecog.recognize();

    if (!playerResult.valid) {
        display->showRetry(); // 显示重试提示
        vTaskDelay(pdMS_TO_TICKS(1000));
        currentState = STATE_COUNTDOWN;
        countdownValue = 3;
        stateStartTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        return;
    }

    // 电脑随机手势
    Gesture pcGesture = gameLogic.generatePCGesture();

    // 判定胜负
    GameResult result = gameLogic.determineWinner(playerResult.gesture, pcGesture);

    // 更新比分
    if (result == RESULT_PLAYER_WIN) playerScore++;
    else if (result == RESULT_PC_WIN) pcScore++;

    // 显示结果
    const char* playerEmoji = gameLogic.gestureToEmoji(playerResult.gesture);
    const char* pcEmoji = gameLogic.gestureToEmoji(pcGesture);
    const char* resultStr = gameLogic.resultToString(result);

    display->showResult(playerEmoji, pcEmoji, resultStr, playerScore, pcScore);

    // 声光提示
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
}
```

**Step 2: 编译上传完整测试**

**Step 3: Commit**

---

## Task 10: 优化与调试

- 调整置信度阈值
- 优化手势识别算法
- 添加日志调试
- 硬件接线确认

---

## 实施完成检查清单

- [ ] 项目结构创建
- [ ] PlatformIO 配置
- [ ] OLED 显示模块
- [ ] 按键输入模块
- [ ] 有限状态机
- [ ] 游戏逻辑
- [ ] 蜂鸣器音效
- [ ] LED 指示
- [ ] 手势识别
- [ ] 完整集成测试
