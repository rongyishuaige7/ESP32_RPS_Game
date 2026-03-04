#ifndef HAND_RECOGNITION_H
#define HAND_RECOGNITION_H

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
    int countFingers();
    Gesture fingersToGesture(int fingerCount);
};

#endif
