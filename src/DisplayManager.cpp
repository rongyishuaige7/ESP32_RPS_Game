#include "DisplayManager.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

DisplayManager::DisplayManager()
    : u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE),
      dirty(true) {}

void DisplayManager::clearScreen() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void DisplayManager::init() {
    u8g2.begin();
    u8g2.setContrast(255);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    dirty = true;
    Serial.printf("[Display] OLED initialized (SDA=%d, SCL=%d)\n", OLED_SDA, OLED_SCL);
}

void DisplayManager::clear() {
    clearScreen();
    dirty = true;
}

void DisplayManager::showIdle() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 28, "RPS Game");
    u8g2.drawStr(10, 48, "Press START");
    u8g2.sendBuffer();
    Serial.println("[Display] Idle: Press START");
}

void DisplayManager::showCountdown(int seconds) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_10x20_tf);
    if (seconds > 0) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", seconds);
        int w = u8g2.getUTF8Width(buf);
        u8g2.drawStr((128 - w) / 2, 40, buf);
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(36, 58, "Get ready!");
    } else {
        const char* goStr = "GO!";
        int w = u8g2.getUTF8Width(goStr);
        u8g2.drawStr((128 - w) / 2, 40, goStr);
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(28, 58, "Show hand!");
    }
    u8g2.sendBuffer();
    if (seconds > 0) Serial.printf("[Display] Countdown: %d\n", seconds);
    else Serial.println("[Display] GO!");
}

void DisplayManager::showPlaying() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(8, 30, "Show your hand");
    u8g2.drawStr(18, 48, "Recognizing...");
    u8g2.sendBuffer();
    Serial.println("[Display] Recognizing gesture...");
}

void DisplayManager::showPCGesture(const char* gestureName) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    const char* label = "PC chose:";
    int w1 = u8g2.getUTF8Width(label);
    u8g2.drawStr((128 - w1) / 2, 28, label);
    u8g2.setFont(u8g2_font_10x20_tf);
    int w2 = u8g2.getUTF8Width(gestureName);
    u8g2.drawStr((128 - w2) / 2, 50, gestureName);
    u8g2.sendBuffer();
    Serial.printf("[Display] PC chose: %s\n", gestureName);
}

void DisplayManager::showResult(const char* playerGesture, const char* pcGesture,
                                const char* result, int playerScore, int pcScore) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    // 第一行：你: X  电脑: Y
    char line1[32];
    snprintf(line1, sizeof(line1), "You:%s  PC:%s", playerGesture, pcGesture);
    u8g2.drawStr(4, 14, line1);
    // 第二行：结果文字
    u8g2.setFont(u8g2_font_10x20_tf);
    int w = u8g2.getUTF8Width(result);
    u8g2.drawStr((128 - w) / 2, 42, result);
    // 第三行：比分
    u8g2.setFont(u8g2_font_6x10_tf);
    char scoreBuf[24];
    snprintf(scoreBuf, sizeof(scoreBuf), "Score %d : %d", playerScore, pcScore);
    w = u8g2.getUTF8Width(scoreBuf);
    u8g2.drawStr((128 - w) / 2, 58, scoreBuf);
    u8g2.sendBuffer();
    Serial.printf("[Display] You:%s PC:%s | %s (%d:%d)\n",
                  playerGesture, pcGesture, result, playerScore, pcScore);
}

void DisplayManager::showRetry() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(8, 28, "Low confidence");
    u8g2.drawStr(18, 46, "Try Again!");
    u8g2.sendBuffer();
    Serial.println("[Display] Low confidence - Try Again!");
}
