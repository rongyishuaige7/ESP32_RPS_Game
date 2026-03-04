#include "AudioManager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "Audio";

AudioManager::AudioManager() {}

void AudioManager::init() {
    // Configure GPIO for buzzer
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    // Configure LEDC timer
    ledc_timer_config_t timer;
    timer.speed_mode = LEDC_LOW_SPEED_MODE;
    timer.timer_num = LEDC_TIMER_0;
    timer.clk_cfg = LEDC_AUTO_CLK;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.freq_hz = 1000;
    ledc_timer_config(&timer);

    // Configure LEDC channel
    ledc_channel_config_t channel;
    channel.gpio_num = BUZZER_PIN;
    channel.speed_mode = LEDC_LOW_SPEED_MODE;
    channel.channel = LEDC_CHANNEL_0;
    channel.timer_sel = LEDC_TIMER_0;
    channel.duty = 0;
    channel.hpoint = 0;
    ledc_channel_config(&channel);
    ledc_fade_func_install(0);

    ESP_LOGI(TAG, "Buzzer initialized on GPIO %d", BUZZER_PIN);
}

void AudioManager::playSound(SoundType type) {
    switch(type) {
        case SOUND_COUNTDOWN:
            beep(1000, 100);
            break;
        case SOUND_PLAYER_WIN:
            // Rising: Do-Re-Do
            beep(523, 150);  // C5
            vTaskDelay(pdMS_TO_TICKS(100));
            beep(587, 150);  // D5
            vTaskDelay(pdMS_TO_TICKS(100));
            beep(523, 200);  // C5
            break;
        case SOUND_PC_WIN:
            // Falling
            beep(587, 150);  // D5
            vTaskDelay(pdMS_TO_TICKS(100));
            beep(523, 250);  // C5
            break;
        case SOUND_DRAW:
            // Double beep: Do-Mi
            beep(523, 100);  // C5
            vTaskDelay(pdMS_TO_TICKS(50));
            beep(659, 150);  // E5
            break;
    }
}

void AudioManager::beep(uint32_t freq, uint32_t duration_ms) {
    // Use PWM to generate tone
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);  // 50% duty
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void AudioManager::stop() {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
