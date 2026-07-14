#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <U8g2lib.h>
#include <stdint.h>

// 0.96 寸 SSD1306 128x64，软件 I2C 独立引脚（避开摄像头 SCCB 4/5）
#define OLED_SDA 1
#define OLED_SCL 3

class DisplayManager {
public:
    DisplayManager();
    void init();
    void showIdle();
    void showCountdown(int seconds);
    void showPlaying();
    void showPCGesture(const char* gestureName);
    void showResult(const char* playerGesture, const char* pcGesture,
                    const char* result, int playerScore, int pcScore);
    void showRetry();
    void clear();

    // 进入某状态时设为 true 请求刷新一次，绘制后清除
    void setDirty(bool d) { dirty = d; }
    bool isDirty() const { return dirty; }

private:
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2;
    bool dirty;

    void clearScreen();
};

#endif
