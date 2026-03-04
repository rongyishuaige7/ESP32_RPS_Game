#include "DisplayManager.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "esp_log.h"

static const char* TAG = "Display";

// SSD1306 Commands
static const uint8_t CMD_DISPLAY_OFF = 0xAE;
static const uint8_t CMD_DISPLAY_ON = 0xAF;
static const uint8_t CMD_SET_CONTRAST = 0x81;
static const uint8_t CMD_ENTIRE_DISPLAY_ON = 0xA4;
static const uint8_t CMD_NORMAL_DISPLAY = 0xA6;
static const uint8_t CMD_SET_MULTIPLEX = 0xA8;
static const uint8_t CMD_SET_DISPLAY_OFFSET = 0xD3;
static const uint8_t CMD_SET_DISPLAY_CLOCK = 0xD5;
static const uint8_t CMD_SET_PRECHARGE = 0xD9;
static const uint8_t CMD_SET_COMPINS = 0xDA;
static const uint8_t CMD_SET_VCOMH = 0xDB;
static const uint8_t CMD_SET_PAGE_START = 0xB0;

DisplayManager::DisplayManager() : cursor_col(0), cursor_row(0) {}

void DisplayManager::i2c_write(i2c_port_t port, uint8_t addr, uint8_t* data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void DisplayManager::i2c_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (gpio_num_t)4,
        .scl_io_num = (gpio_num_t)15,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = { .clk_speed = 400000 },
        .clk_flags = 0
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void DisplayManager::sendCommand(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write(I2C_NUM_0, SSD1306_ADDR, buf, 2);
}

void DisplayManager::sendData(uint8_t* data, size_t len) {
    uint8_t* buf = (uint8_t*)malloc(len + 1);
    buf[0] = 0x40;
    memcpy(&buf[1], data, len);
    i2c_write(I2C_NUM_0, SSD1306_ADDR, buf, len + 1);
    free(buf);
}

void DisplayManager::init() {
    i2c_init();

    // SSD1306 Initialization sequence
    sendCommand(CMD_DISPLAY_OFF);
    sendCommand(CMD_SET_DISPLAY_CLOCK); sendCommand(0x80);
    sendCommand(CMD_SET_MULTIPLEX); sendCommand(0x3F);
    sendCommand(CMD_SET_DISPLAY_OFFSET); sendCommand(0x00);
    sendCommand(CMD_SET_PAGE_START | 0x00);
    sendCommand(CMD_SET_COMPINS); sendCommand(0x12);
    sendCommand(CMD_SET_VCOMH); sendCommand(0x40);
    sendCommand(CMD_SET_PRECHARGE); sendCommand(0x22);
    sendCommand(CMD_ENTIRE_DISPLAY_ON);
    sendCommand(CMD_NORMAL_DISPLAY);
    sendCommand(CMD_SET_CONTRAST); sendCommand(0xFF);
    sendCommand(CMD_DISPLAY_ON);

    clearScreen();
    ESP_LOGI(TAG, "OLED initialized");
}

void DisplayManager::clearScreen() {
    uint8_t zero[128];
    memset(zero, 0, 128);

    for (int page = 0; page < 8; page++) {
        sendCommand(CMD_SET_PAGE_START | page);
        sendCommand(0x00); // Column start
        sendCommand(0x10); // Column start high
        sendData(zero, 128);
    }
}

void DisplayManager::showIdle() {
    clearScreen();
    ESP_LOGI(TAG, "Show idle: Waiting... Press START");
}

void DisplayManager::showCountdown(int seconds) {
    char buf[16];
    snprintf(buf, sizeof(buf), "Countdown: %d", seconds);
    clearScreen();
    ESP_LOGI(TAG, "%s", buf);
}

void DisplayManager::showPlaying() {
    clearScreen();
    ESP_LOGI(TAG, "Recognizing...");
}

void DisplayManager::showResult(const char* playerGesture, const char* pcGesture,
                                const char* result, int playerScore, int pcScore) {
    char line1[32];
    char line2[32];
    snprintf(line1, sizeof(line1), "You:%s PC:%s", playerGesture, pcGesture);
    snprintf(line2, sizeof(line2), "%s (%d:%d)", result, playerScore, pcScore);

    clearScreen();
    ESP_LOGI(TAG, "%s", line1);
    ESP_LOGI(TAG, "%s", line2);
}

void DisplayManager::showRetry() {
    clearScreen();
    ESP_LOGI(TAG, "Try Again! Low confidence");
}

void DisplayManager::clear() {
    clearScreen();
}

void DisplayManager::setCursor(uint8_t col, uint8_t row) {
    cursor_col = col;
    cursor_row = row;
}

void DisplayManager::print(const char* str) {
    (void)str;
    // Would need font data for full implementation
}
