# ESP32 RPS Game

基于 ESP32-S3 的猜拳游戏：摄像头识别手势（石头/剪刀/布），与电脑随机出拳对决，结果通过 OLED、蜂鸣器和 RGB LED 反馈；可选 WiFi 实时摄像头流。

## 项目照片与资料

相关 EDA 文件已整理，文件处理说明见 [MEDIA_EVIDENCE](docs/MEDIA_EVIDENCE.md)。

## 功能

- 手势识别：石头（握拳）、剪刀（两指）、布（张开手掌），纯图像处理（YCbCr 肤色 + 指峰检测）
- 四状态流程：待机 → 倒计时 → 出拳识别 → 结果展示，支持多局计分
- OLED 显示当前状态与比分；蜂鸣器播放倒计时/胜负音效；RGB LED 指示胜负（绿/红/黄）
- 可选 HTTP MJPEG 摄像头流，在待机或结果界面可浏览器查看画面

## 硬件清单

| 组件 | 说明 |
|------|------|
| 主控 | ESP32-S3（需 PSRAM，如 ESP32-S3-DevKitC-1、ESP32-S3-EYE 等） |
| 摄像头 | 板载 OV3660（CSI），或兼容引脚的外接模组 |
| 显示屏 | 0.96" SSD1306 OLED，128×64，I2C |
| 按键 | 2 个：开始、复位 |
| 蜂鸣器 | 无源压电蜂鸣器 |
| LED | 共阳极 RGB LED（红/绿/蓝） |

## 引脚接线

| 功能 | GPIO | 说明 |
|------|------|------|
| **OLED (I2C)** | | 软件 I2C，与摄像头 SCCB 分离 |
| SDA | 1 | 数据 |
| SCL | 3 | 时钟 |
| **按键** | | 低电平有效，内部上拉，一端接 GND |
| 开始 | 0 | 开始游戏 / 进入倒计时 |
| 复位 | 47 | 清零比分并返回待机 |
| **蜂鸣器** | 2 | 正极接 GPIO 2，负极 GND |
| **RGB LED（共阳）** | | 低电平点亮，各色串 220–330Ω |
| 红 | 48 | 电脑赢 |
| 绿 | 45 | 玩家赢 |
| 蓝 | 38 | 预留；平局为红+绿同亮（黄） |

摄像头引脚为板载常用配置（XCLK=15, SIOD=4, SIOC=5, D0–D7, VSYNC/HREF/PCLK 等），见 `include/HandRecognition.h`。OLED 使用 1/3 与摄像头 4/5 分开，避免总线冲突。

## 构建与烧录

需要 [PlatformIO](https://platformio.org/)（CLI 或 VS Code 插件）。

```bash
# 编译
pio run

# 烧录
pio run --target upload

# 串口监视（波特率 115200）
pio device monitor -b 115200
```

板型与 PSRAM 等在 `platformio.ini` 中以 `esp32-s3-devkitc-1` 兼容配置构建。当前配置启用了 OPI PSRAM；请确认你的实际模组确实带 PSRAM。若使用无 PSRAM 的 N8 或其他板型，需要调整 `board` 和内存配置，不能只依赖项目名称判断。

## 游戏玩法

1. **待机**：OLED 显示 “RPS Game / Press START”。按 **开始键** 进入倒计时。
2. **倒计时**：3→2→1→GO!，蜂鸣器每秒一响，结束后进入出拳。
3. **出拳**：将手放入画面中心区域，做石头/剪刀/布。若识别不稳定会提示重试并再次倒计时。
4. **结果**：显示双方出拳与胜负，蜂鸣器与 LED 反馈，约 2 秒后自动进入下一局倒计时。
5. 任意时刻按 **复位键** 可清零比分并回到待机。

手势约定：握拳 = 石头，两指 = 剪刀，张开手掌 = 布。识别仅用中心约 60% 画面，减少背景干扰。

## WiFi 摄像头流（可选）

固件默认以 **AP 模式** 建热点，仅在 **待机** 或 **结果** 界面推流，避免与识别争用摄像头。

- **热点**：SSID `RPS-CAM`，密码 `12345678`
- 连接后浏览器打开：**http://192.168.4.1/** 或直接 **http://192.168.4.1/stream** 观看 MJPEG 流

帧率默认约 5 fps，可在 `include/CameraStream.h` 中修改 `STREAM_FPS`。若需连接已有路由器（STA 模式），见 [`DEPLOYMENT.md`](./DEPLOYMENT.md)。WiFi 凭据必须通过本地构建参数提供，不要提交到 Git。

> 默认 AP 密码仅用于本地原型测试。将设备交给他人或在公共场所使用前，请改成独立强密码。

## 项目结构

```
ESP32_RPS_Game/
├── platformio.ini       # 板型、框架、库依赖、编译选项
├── src/
│   ├── main.cpp         # 入口：初始化各模块，主循环调用状态机与音频
│   ├── StateMachine.cpp # 四状态 FSM：IDLE / COUNTDOWN / PLAYING / RESULT
│   ├── GameLogic.cpp    # 猜拳胜负与随机出拳
│   ├── HandRecognition.cpp  # 摄像头 + 肤色分割 + 指峰识别
│   ├── DisplayManager.cpp   # U8g2 OLED 各状态界面
│   ├── CameraStream.cpp     # WiFi AP/STA + HTTP MJPEG 服务
│   ├── AudioManager.cpp     # 蜂鸣器非阻塞音效
│   ├── ButtonManager.cpp    # 按键消抖
│   └── LEDManager.cpp       # RGB LED
├── include/             # 对应头文件与引脚/参数宏
└── DEPLOYMENT.md        # 详细接线、环境、FAQ（中文）
```

额外依赖库（由 PlatformIO 安装）：`olikraus/U8g2`。摄像头驱动由当前 Arduino-ESP32 框架提供。
