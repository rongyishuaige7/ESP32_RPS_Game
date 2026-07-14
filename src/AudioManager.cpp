#include "AudioManager.h"
#include <Arduino.h>

AudioManager::AudioManager()
    : sequenceLen(0), currentNote(0), noteStartMs(0), playing(false) {}

void AudioManager::init() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);
    stopTone();
    Serial.println("Buzzer initialized (LEDC) on GPIO " + String(BUZZER_PIN));
}

void AudioManager::startTone(uint32_t freq_hz) {
    if (freq_hz > 0) {
        ledcWriteTone(LEDC_CHANNEL, freq_hz);
    } else {
        ledcWriteTone(LEDC_CHANNEL, 0);
    }
}

void AudioManager::stopTone() {
    ledcWriteTone(LEDC_CHANNEL, 0);
}

void AudioManager::stop() {
    stopTone();
    playing = false;
}

void AudioManager::playSound(SoundType type) {
    stopTone();
    sequenceLen = 0;
    switch (type) {
        case SOUND_COUNTDOWN:
            sequence[0] = {1000, 100};
            sequenceLen = 1;
            break;
        case SOUND_GO:
            sequence[0] = {440, 80};
            sequence[1] = {0, 40};
            sequence[2] = {880, 120};
            sequenceLen = 3;
            break;
        case SOUND_PLAYER_WIN:
            sequence[0] = {523, 150};
            sequence[1] = {0, 100};   // 间隔
            sequence[2] = {587, 150};
            sequence[3] = {0, 100};
            sequence[4] = {523, 200};
            sequenceLen = 5;
            break;
        case SOUND_PC_WIN:
            sequence[0] = {587, 150};
            sequence[1] = {0, 100};
            sequence[2] = {523, 250};
            sequenceLen = 3;
            break;
        case SOUND_DRAW:
            sequence[0] = {523, 100};
            sequence[1] = {0, 50};
            sequence[2] = {659, 150};
            sequenceLen = 3;
            break;
    }
    if (sequenceLen > 0) {
        currentNote = 0;
        noteStartMs = millis();
        startTone(sequence[0].freq_hz);
        playing = true;
    }
}

void AudioManager::update() {
    if (!playing || sequenceLen == 0) return;

    uint32_t now = millis();
    uint32_t elapsed = now - noteStartMs;
    if (elapsed < sequence[currentNote].duration_ms) return;

    currentNote++;
    if (currentNote >= sequenceLen) {
        stopTone();
        playing = false;
        return;
    }
    noteStartMs = now;
    startTone(sequence[currentNote].freq_hz);
}
