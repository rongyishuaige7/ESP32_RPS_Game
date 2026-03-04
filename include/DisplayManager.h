#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "driver/i2c.h"
#include "driver/gpio.h"

class DisplayManager {
public:
    DisplayManager();
    void init();
    void showIdle();
    void showCountdown(int seconds);
    void showPlaying();
    void showResult(const char* playerGesture, const char* pcGesture,
                    const char* result, int playerScore, int pcScore);
    void showRetry();
    void clear();

private:
    static const uint8_t SSD1306_ADDR = 0x3C;
    void i2c_init();
    void i2c_write(i2c_port_t port, uint8_t addr, uint8_t* data, size_t len);
    void sendCommand(uint8_t cmd);
    void sendData(uint8_t* data, size_t len);
    void clearScreen();
    void setCursor(uint8_t col, uint8_t row);
    void print(const char* str);
    uint8_t cursor_col;
    uint8_t cursor_row;
};

#endif
