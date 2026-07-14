#include <Arduino.h>
#include "DisplayManager.h"
#include "ButtonManager.h"
#include "StateMachine.h"
#include "GameLogic.h"
#include "HandRecognition.h"
#include "AudioManager.h"
#include "LEDManager.h"
#include "CameraStream.h"

// 全局模块实例
DisplayManager display;
ButtonManager buttons;
GameLogic gameLogic;
HandRecognition handRecog;
AudioManager audio;
LEDManager led;
StateMachine stateMachine(&display, &buttons, &audio, &led, &handRecog, &gameLogic);
CameraStream cameraStream;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("RPS Game Starting...");

    // 先初始化摄像头（其 SCCB 需在 OLED I2C 之前完成）
    handRecog.init();

    // 再初始化显示屏及其他模块
    display.init();
    buttons.init();
    audio.init();
    led.init();

    // 从待机状态开始
    stateMachine.reset();

    // WiFi + HTTP MJPEG 推流（仅在待机/结果界面推流，约 5 fps）
    if (handRecog.isReady()) {
        cameraStream.start(&stateMachine);
    } else {
        Serial.println("[CameraStream] Camera unavailable; stream server not started");
    }

    Serial.println("Setup complete!");
}

void loop() {
    audio.update();
    stateMachine.update();
    delay(50);
}
