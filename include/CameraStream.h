#ifndef CAMERA_STREAM_H
#define CAMERA_STREAM_H

class StateMachine;

/**
 * WiFi HTTP MJPEG 推流服务。
 * - 仅在游戏状态为 IDLE 或 RESULT 时推帧（倒计时/识别过程中不推）。
 * - 帧率由 STREAM_FPS 限制（默认 5），以降低 CPU/内存占用。
 * - 默认 AP 模式；设 WIFI_USE_STA 及 WIFI_SSID/WIFI_PASS 可改为 STA 连接路由器。
 */
class CameraStream {
public:
    CameraStream();
    /** 启动 WiFi 与 HTTP 服务；需在摄像头初始化之后调用。 */
    void start(StateMachine* stateMachine);
    /** 推流服务是否已启动。 */
    bool isRunning() const { return running; }

private:
    bool running;
};

// 配置（可在 platformio.ini 或此处覆盖）
#ifndef WIFI_AP_SSID
#define WIFI_AP_SSID "RPS-CAM"
#endif
#ifndef WIFI_AP_PASS
#define WIFI_AP_PASS "12345678"
#endif
#ifndef WIFI_USE_STA
#define WIFI_USE_STA 0
#endif
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif
#ifndef STREAM_FPS
#define STREAM_FPS 5
#endif

#endif
