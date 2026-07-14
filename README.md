# ESP32-S3 Rock Paper Scissors

一个基于 ESP32-S3、ESP-IDF 和 PlatformIO 的剪刀石头布硬件游戏原型。项目把按键、倒计时、OLED、蜂鸣器、RGB LED、比分与状态机拆分成独立模块，适合用于嵌入式游戏流程和外设联调实验。

> **重要状态说明：** 当前公开版本的完整游戏状态机可以运行，但“手势识别”仍是测试桩：`HandRecognition::recognize()` 使用随机数返回石头、剪刀或布，尚未读取摄像头图像。不要把当前版本描述为已完成真实视觉识别。

## 当前已实现

- `IDLE → COUNTDOWN → PLAYING → RESULT` 游戏状态机
- 开始和复位按键输入
- 电脑手势随机生成与胜负判断
- 连续比分记录
- OLED 初始化和显示流程接口
- 蜂鸣器倒计时及不同结果音效
- RGB LED 胜 / 负 / 平反馈
- PlatformIO + ESP-IDF 工程配置

## 尚未完成

- 摄像头采集和真实手势识别
- 指尖/轮廓算法或 TFLite 模型
- OLED 字库和实际文本绘制：当前显示函数主要通过串口日志输出状态
- 真机接线图、实物照片和硬件在环测试报告

## 硬件目标

- ESP32-S3 开发板（当前 PlatformIO board：`esp32-s3-devkitc-1`）
- SSD1306 I²C OLED（地址 `0x3C`）
- 开始按键、复位按键
- 蜂鸣器
- RGB LED
- 摄像头模块（为后续真实识别预留，当前未接入代码）

## 当前引脚

以下引脚来自当前源码，是待真机核对的实现值：

| 功能 | GPIO |
| --- | --- |
| OLED SDA | GPIO 4 |
| OLED SCL | GPIO 15 |
| 开始按键 | GPIO 0 |
| 复位按键 | GPIO 47 |
| 蜂鸣器 | GPIO 21 |
| LED 绿 | GPIO 45 |
| LED 红 | GPIO 48 |
| LED 蓝 | GPIO 38 |

> 不同 ESP32-S3 板卡的可用引脚、启动脚和板载外设不同。接线前请核对具体板卡原理图；不要仅凭本表直接通电。

## 架构

```text
src/
├── main.cpp               # 初始化与主循环
├── StateMachine.cpp       # 游戏状态流转
├── GameLogic.cpp          # 随机电脑手势、胜负判断
├── HandRecognition.cpp    # 当前为随机手势测试桩
├── DisplayManager.cpp     # SSD1306 初始化与显示接口
├── ButtonManager.cpp      # 按键中断
├── AudioManager.cpp       # LEDC 蜂鸣器
└── LEDManager.cpp         # 结果灯效
```

## 构建

### VS Code + PlatformIO

1. 安装 [PlatformIO](https://platformio.org/)。
2. 打开仓库根目录。
3. 选择环境 `esp32-s3-devkitc-1`。
4. 依次执行 Build、Upload、Monitor。

### PlatformIO CLI

```bash
pio run
pio run --target upload
pio device monitor --baud 115200
```

项目当前锁定：

```ini
platform = espressif32@6.4.0
framework = espidf
```

## 当前游戏流程

1. 启动后进入等待状态。
2. 按下开始键，进入 3 秒倒计时。
3. `HandRecognition` 返回测试手势。
4. `GameLogic` 随机生成电脑手势并判断结果。
5. OLED 接口、串口日志、蜂鸣器和 LED 给出反馈。
6. 两秒后自动开始下一轮；复位键可随时清零比分。

## 把测试桩升级为真实识别

建议按以下顺序推进：接入并验证 `esp32-camera`、建立带标签测试集、选择传统图像处理或量化模型、定义置信度和重试行为，并在不同光照/距离/背景下做真机测试。

## 已知限制

- 公开代码未完成真实视觉识别。
- OLED 文本字模尚未实现，主要状态目前见串口日志。
- 引脚和外设组合尚缺少公开真机验证证据。
- 没有 CI、单元测试、Release 或预编译固件。

## License

本项目采用 [MIT License](./LICENSE)。
