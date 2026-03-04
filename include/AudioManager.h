#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "driver/ledc.h"
#include "driver/gpio.h"

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
};

#endif
