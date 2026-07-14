#ifndef HAND_RECOGNITION_H
#define HAND_RECOGNITION_H

#include "GameLogic.h"
#include <stdint.h>

struct RecognitionResult {
    Gesture gesture;
    float confidence;
    bool valid;
};

// ===== ESP32-S3 + OV3660 摄像头引脚配置 =====
// 适用于 ESP32-S3-EYE / Freenove ESP32-S3 CAM 等板型
// 若使用其他板子请按实际引脚修改
#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5
#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

// 处理参数
#define PROC_WIDTH      160     // 下采样后的处理宽度
#define PROC_HEIGHT     120     // 下采样后的处理高度
#define SKIN_MIN_RATIO  0.05f   // 有效手部的最小肤色像素占比
#define SKIN_MAX_RATIO  0.80f   // 最大肤色占比（避免误检）
#define CONF_THRESHOLD  0.60f   // 接受结果的最低置信度
#define RECOG_NUM_FRAMES 3      // 多数投票的帧数
#define ROI_PERCENT     60      // 仅处理画面中心 60%（左右各 20% 边距）

class HandRecognition {
public:
    HandRecognition();
    void init();
    RecognitionResult recognize();
    bool isReady() const { return cameraReady; }

private:
    bool cameraReady;

    bool initCamera();
    bool detectSkinColor(uint8_t r, uint8_t g, uint8_t b);
    void rgb565_to_rgb(uint16_t pixel, uint8_t* r, uint8_t* g, uint8_t* b);
    int analyzeFrame(uint8_t* fbBuf, int width, int height, float* outConfidence);
    Gesture fingersToGesture(int fingerCount);
    RecognitionResult recognizeOneFrame();
};

#endif
