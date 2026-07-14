#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "driver/gpio.h"
#include <stdint.h>

enum SoundType {
    SOUND_COUNTDOWN,
    SOUND_GO,         // 倒计时到 0 时播放，进入出拳前
    SOUND_PLAYER_WIN,
    SOUND_PC_WIN,
    SOUND_DRAW
};

#define AUDIO_MAX_NOTES 8

struct AudioNote {
    uint32_t freq_hz;
    uint32_t duration_ms;
};

class AudioManager {
public:
    AudioManager();
    void init();
    void playSound(SoundType type);
    void update();  // 在 loop() 中调用，非阻塞播放
    void stop();

private:
    static const int BUZZER_PIN = 2;
    static const int LEDC_CHANNEL = 1;  // 用 1 避免与摄像头 XCLK（通道 0）冲突

    AudioNote sequence[AUDIO_MAX_NOTES];
    int sequenceLen;
    int currentNote;
    uint32_t noteStartMs;
    bool playing;

    void startTone(uint32_t freq_hz);
    void stopTone();
};

#endif
