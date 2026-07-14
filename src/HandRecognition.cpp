#include "HandRecognition.h"
#include <Arduino.h>
#include <esp_camera.h>
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char* TAG = "HandRecog";

HandRecognition::HandRecognition() : cameraReady(false) {}

// =====================================================================
// 摄像头初始化（ESP32-S3 + OV3660）
// =====================================================================
bool HandRecognition::initCamera() {
    // 新版 esp32-camera 在结构尾部增加了字段；零初始化可避免未赋值字段
    // 带入随机栈数据，导致不同框架版本下出现非确定性初始化失败。
    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = CAM_PIN_D0;
    config.pin_d1       = CAM_PIN_D1;
    config.pin_d2       = CAM_PIN_D2;
    config.pin_d3       = CAM_PIN_D3;
    config.pin_d4       = CAM_PIN_D4;
    config.pin_d5       = CAM_PIN_D5;
    config.pin_d6       = CAM_PIN_D6;
    config.pin_d7       = CAM_PIN_D7;
    config.pin_xclk     = CAM_PIN_XCLK;
    config.pin_pclk     = CAM_PIN_PCLK;
    config.pin_vsync    = CAM_PIN_VSYNC;
    config.pin_href     = CAM_PIN_HREF;
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;
    config.pin_pwdn     = CAM_PIN_PWDN;
    config.pin_reset    = CAM_PIN_RESET;
    config.xclk_freq_hz = 20000000;        // 20MHz 主时钟
    config.pixel_format = PIXFORMAT_RGB565; // 用于色彩分析
    config.frame_size   = FRAMESIZE_QVGA;   // 320x240
    config.jpeg_quality = 12;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[HandRecog] Camera init FAILED: 0x%x\n", err);
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        return false;
    }

    // OV3660 传感器参数微调
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 1);
        s->set_contrast(s, 1);
        s->set_saturation(s, 0);
        s->set_whitebal(s, 1);
        s->set_awb_gain(s, 1);
        s->set_exposure_ctrl(s, 1);
        s->set_gain_ctrl(s, 1);
        s->set_hmirror(s, 0);
        s->set_vflip(s, 0);
        ESP_LOGI(TAG, "OV3660 sensor configured");
    }

    // 丢弃前几帧以完成自动曝光预热
    for (int i = 0; i < 5; i++) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    Serial.println("[HandRecog] Camera initialized successfully!");
    ESP_LOGI(TAG, "Camera initialized successfully");
    return true;
}

void HandRecognition::init() {
    Serial.println("[HandRecog] Initializing camera (OV3660)...");
    cameraReady = initCamera();
    if (cameraReady) {
        Serial.println("[HandRecog] Camera ready!");
    } else {
        Serial.println("[HandRecog] Camera FAILED - recognition disabled until restart");
    }
}

// =====================================================================
// RGB565 转 RGB
// =====================================================================
void HandRecognition::rgb565_to_rgb(uint16_t pixel, uint8_t* r, uint8_t* g, uint8_t* b) {
    // ESP32 小端、摄像头大端，需交换字节
    pixel = (pixel >> 8) | (pixel << 8);
    *r = ((pixel >> 11) & 0x1F) << 3;
    *g = ((pixel >> 5)  & 0x3F) << 2;
    *b = (pixel & 0x1F) << 3;
}

// =====================================================================
// YCbCr 肤色检测
// =====================================================================
bool HandRecognition::detectSkinColor(uint8_t r, uint8_t g, uint8_t b) {
    // RGB 转 YCbCr
    int Y  = (int)( 0.299f * r + 0.587f * g + 0.114f * b);
    int Cb = (int)(128.0f - 0.169f * r - 0.331f * g + 0.500f * b);
    int Cr = (int)(128.0f + 0.500f * r - 0.419f * g - 0.081f * b);

    // 经验肤色阈值（适配多种肤色）
    return (Y > 50 && Cr >= 130 && Cr <= 180 && Cb >= 70 && Cb <= 130);
}

// =====================================================================
// 帧分析：肤色分割 + 直方图指峰计数
// 仅处理画面中心 ROI_PERCENT% 区域以降低背景干扰。
// 返回检测到的指峰数，未检测到手则返回 -1
// =====================================================================
int HandRecognition::analyzeFrame(uint8_t* fbBuf, int width, int height, float* outConfidence) {
    int procW = width / 2;
    int procH = height / 2;
    if (procW > PROC_WIDTH) procW = PROC_WIDTH;
    if (procH > PROC_HEIGHT) procH = PROC_HEIGHT;

    // 感兴趣区域：画面中心 60%（左右各留 20% 边距）
    int marginX = (100 - ROI_PERCENT) * procW / 200;
    int marginY = (100 - ROI_PERCENT) * procH / 200;
    int roiX0 = marginX;
    int roiX1 = procW - marginX;
    int roiY0 = marginY;
    int roiY1 = procH - marginY;
    int roiW = roiX1 - roiX0;
    int roiH = roiY1 - roiY0;
    int totalPixels = roiW * roiH;
    if (totalPixels <= 0) {
        *outConfidence = 0.0f;
        return -1;
    }

    int colHist[PROC_WIDTH];
    memset(colHist, 0, sizeof(int) * procW);

    int skinCount = 0;
    int minX = procW, maxX = 0, minY = procH, maxY = 0;

    uint16_t* pixels = (uint16_t*)fbBuf;

    // --- 第一遍：仅在 ROI 内做肤色分割，2 倍下采样 ---
    for (int y = roiY0; y < roiY1; y++) {
        for (int x = roiX0; x < roiX1; x++) {
            uint16_t pixel = pixels[(y * 2) * width + (x * 2)];
            uint8_t r, g, b;
            rgb565_to_rgb(pixel, &r, &g, &b);

            if (detectSkinColor(r, g, b)) {
                colHist[x]++;
                skinCount++;
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }

    float skinRatio = (float)skinCount / totalPixels;
    ESP_LOGI(TAG, "Skin ratio: %.2f%%, bbox: [%d,%d]-[%d,%d]",
             skinRatio * 100.0f, minX, minY, maxX, maxY);

    if (skinRatio < SKIN_MIN_RATIO) {
        ESP_LOGW(TAG, "Not enough skin detected (%.1f%%)", skinRatio * 100.0f);
        *outConfidence = skinRatio;
        return -1;
    }
    if (skinRatio > SKIN_MAX_RATIO) {
        ESP_LOGW(TAG, "Too much skin / bad background (%.1f%%)", skinRatio * 100.0f);
        *outConfidence = 0.3f;
        return -1;
    }

    // --- 第二遍：构建手指区域直方图（手上部 1/3） ---
    int handHeight = maxY - minY;
    if (handHeight < 10) {
        *outConfidence = 0.2f;
        return -1;
    }

    int fingerTop = minY;
    int fingerBot = minY + handHeight / 3;
    int handWidth = maxX - minX;
    if (handWidth < 5) {
        *outConfidence = 0.2f;
        return -1;
    }

    int fingerHist[PROC_WIDTH];
    memset(fingerHist, 0, sizeof(int) * procW);

    for (int y = fingerTop; y <= fingerBot; y++) {
        for (int x = minX; x <= maxX; x++) {
            uint16_t pixel = pixels[(y * 2) * width + (x * 2)];
            uint8_t r, g, b;
            rgb565_to_rgb(pixel, &r, &g, &b);
            if (detectSkinColor(r, g, b)) {
                fingerHist[x]++;
            }
        }
    }

    // --- 直方图平滑（移动平均，核宽 5） ---
    int smoothed[PROC_WIDTH];
    memset(smoothed, 0, sizeof(int) * procW);
    for (int x = minX; x <= maxX; x++) {
        int sum = 0, cnt = 0;
        for (int k = -2; k <= 2; k++) {
            int idx = x + k;
            if (idx >= minX && idx <= maxX) {
                sum += fingerHist[idx];
                cnt++;
            }
        }
        smoothed[x] = (cnt > 0) ? sum / cnt : 0;
    }

    // --- 带滞回的峰检测 ---
    int maxVal = 0;
    for (int x = minX; x <= maxX; x++) {
        if (smoothed[x] > maxVal) maxVal = smoothed[x];
    }

    if (maxVal < 2) {
        *outConfidence = 0.3f;
        return -1;
    }

    int highTh = maxVal / 3;
    int lowTh  = maxVal / 6;
    int peaks  = 0;
    bool inPeak = false;

    for (int x = minX; x <= maxX; x++) {
        if (!inPeak && smoothed[x] > highTh) {
            peaks++;
            inPeak = true;
        } else if (inPeak && smoothed[x] < lowTh) {
            inPeak = false;
        }
    }

    // --- 谷深分析（区分剪刀与布） ---
    // 真剪刀（两根独立手指）：两峰之间谷较深（比值约 0）。
    // 布被误判为 2 峰（整掌、浅凹）：谷较浅（比值 >0.45）。
    // 取手宽内 60% 区域的最小值，避免边缘衰减影响。
    if (peaks == 2) {
        int innerStart = minX + handWidth / 5;
        int innerEnd   = maxX - handWidth / 5;
        int innerMin   = maxVal;
        for (int x = innerStart; x <= innerEnd; x++) {
            if (smoothed[x] < innerMin) innerMin = smoothed[x];
        }
        float valleyRatio = (maxVal > 0) ? (float)innerMin / (float)maxVal : 0.0f;
        ESP_LOGI(TAG, "2-peak valley ratio: %.2f (%.0f%% skin)", valleyRatio, skinRatio * 100.0f);
        if (valleyRatio >= 0.45f) {
            // 谷浅，说明是整掌占满画面，而非两根分开的手指
            peaks = 3;
            ESP_LOGI(TAG, "Reclassified 2 peaks -> paper (shallow valley)");
        }
    }

    ESP_LOGI(TAG, "Detected %d peak(s) in finger region", peaks);

    // --- 置信度计算 ---
    float conf = 0.5f;
    // 肤色占比合理时加分
    if (skinRatio >= 0.10f && skinRatio <= 0.60f) conf += 0.15f;
    // 仅对典型峰数加分（0–1 石头、2 剪刀、3 布）；4+ 视为歧义
    if (peaks <= 1) conf += 0.15f;       // 石头
    else if (peaks == 2) conf += 0.15f;  // 剪刀
    else if (peaks == 3) conf += 0.15f;  // 布
    // 峰较强时加分
    if (maxVal > (fingerBot - fingerTop) / 2) conf += 0.1f;
    // 肤色几乎占满 ROI 时扣分：手太近，峰更不可靠
    if (skinRatio > 0.55f) conf -= 0.10f;
    if (conf < 0.0f) conf = 0.0f;
    if (conf > 1.0f) conf = 1.0f;

    *outConfidence = conf;
    return peaks;
}

// =====================================================================
// 指峰数 → 手势映射
// =====================================================================
Gesture HandRecognition::fingersToGesture(int fingerCount) {
    if (fingerCount <= 1) return GESTURE_STONE;     // 0–1 指 → 石头
    if (fingerCount == 2) return GESTURE_SCISSORS;   // 2 指 → 剪刀
    return GESTURE_PAPER;                            // 3 指及以上 → 布
}

// =====================================================================
// 单帧识别（供 recognize() 做多帧投票用）
// =====================================================================
RecognitionResult HandRecognition::recognizeOneFrame() {
    RecognitionResult result = {GESTURE_NONE, 0.0f, false};
    if (!cameraReady) return result;

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb || fb->format != PIXFORMAT_RGB565) {
        if (fb) esp_camera_fb_return(fb);
        return result;
    }

    float confidence = 0.0f;
    int fingerCount = analyzeFrame(fb->buf, fb->width, fb->height, &confidence);
    esp_camera_fb_return(fb);

    if (fingerCount < 0) {
        result.confidence = confidence;
        result.valid = false;
        return result;
    }
    result.gesture    = fingersToGesture(fingerCount);
    result.confidence = confidence;
    result.valid      = (confidence >= CONF_THRESHOLD);
    return result;
}

// =====================================================================
// 主识别入口：3 帧多数投票
// =====================================================================
RecognitionResult HandRecognition::recognize() {
    RecognitionResult result = {GESTURE_NONE, 0.0f, false};

    if (!cameraReady) {
        ESP_LOGE(TAG, "Camera not ready; refusing to fabricate a gesture");
        return result;
    }

    unsigned long t0 = millis();
    RecognitionResult samples[RECOG_NUM_FRAMES];
    int validCount = 0;

    for (int i = 0; i < RECOG_NUM_FRAMES; i++) {
        samples[i] = recognizeOneFrame();
        if (samples[i].valid) validCount++;
    }

    unsigned long elapsed = millis() - t0;
    ESP_LOGI(TAG, "Recognition took %lu ms (%d/%d valid)", elapsed, validCount, RECOG_NUM_FRAMES);

    if (validCount == 0) {
        result.confidence = samples[0].confidence;
        for (int i = 1; i < RECOG_NUM_FRAMES; i++) {
            if (samples[i].confidence > result.confidence)
                result.confidence = samples[i].confidence;
        }
        result.valid = false;
        return result;
    }

    // 对手势做多数投票
    int countStone = 0, countScissors = 0, countPaper = 0;
    for (int i = 0; i < RECOG_NUM_FRAMES; i++) {
        if (!samples[i].valid) continue;
        if (samples[i].gesture == GESTURE_STONE) countStone++;
        else if (samples[i].gesture == GESTURE_SCISSORS) countScissors++;
        else if (samples[i].gesture == GESTURE_PAPER) countPaper++;
    }
    int bestCount = countStone;
    result.gesture = GESTURE_STONE;
    if (countScissors > bestCount) { bestCount = countScissors; result.gesture = GESTURE_SCISSORS; }
    if (countPaper > bestCount)   { bestCount = countPaper;   result.gesture = GESTURE_PAPER;     }

    // 仅对投票给胜出手势的帧求平均置信度，避免被异议帧稀释，
    // 也避免 meanAll * (bestCount/validCount) 因浮点精度略低于 CONF_THRESHOLD 的问题。
    float sumWinConf = 0.0f;
    for (int i = 0; i < RECOG_NUM_FRAMES; i++) {
        if (samples[i].valid && samples[i].gesture == result.gesture)
            sumWinConf += samples[i].confidence;
    }
    result.confidence = sumWinConf / (float)bestCount;

    // 至少 2 帧一致才有效；bestCount >= 2 已保证一致性。
    result.valid = (bestCount >= 2 && result.confidence >= CONF_THRESHOLD);

    const char* names[] = {"NONE", "STONE", "SCISSORS", "PAPER"};
    ESP_LOGI(TAG, "Vote: %s (conf=%.2f, valid=%d)", names[result.gesture], result.confidence, result.valid);
    return result;
}
