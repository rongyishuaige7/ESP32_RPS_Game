# 部署与真机验证

本文档用于 ESP32-S3 + OV3660 原型的本地构建、烧录和验收。仓库不会保存真实 WiFi 凭据。

## 1. 确认硬件

在通电前核对：

- 实际 ESP32-S3 模组带 PSRAM，并与 `qio_opi` 内存配置兼容。
- 摄像头引脚与 `include/HandRecognition.h` 一致。
- OLED 使用 GPIO 1/3，不与摄像头 SCCB GPIO 4/5 冲突。
- 蜂鸣器使用 GPIO 2；共阳极 RGB LED 各颜色串联限流电阻。
- 外设共地，供电电压符合模组规格。

## 2. 构建

```bash
pio run -t clean
pio run
```

成功构建后，固件位于：

```text
.pio/build/esp32-s3-devkitc-1/firmware.bin
```

## 3. 烧录与串口

连接设备后先确认端口：

```bash
pio device list
```

再执行：

```bash
pio run --target upload --upload-port /dev/ttyACM0
pio device monitor --port /dev/ttyACM0 --baud 115200
```

端口可能是 `/dev/ttyUSB0` 或其他名称，以实际枚举结果为准。

## 4. WiFi 模式

默认使用设备 AP：

```text
SSID: RPS-CAM
```

默认密码只适用于本地原型。共享或公开使用前，应通过私有构建配置覆盖：

```ini
build_flags =
    ${env:esp32-s3-devkitc-1.build_flags}
    -DWIFI_AP_SSID=\"your-device-ssid\"
    -DWIFI_AP_PASS=\"your-strong-password\"
```

STA 模式同样通过本地未提交配置设置 `WIFI_USE_STA=1`、`WIFI_SSID` 和 `WIFI_PASS`。不要把真实凭据写入仓库的 `platformio.ini` 或头文件。

## 5. 真机验收

按顺序检查：

1. 串口出现摄像头初始化成功信息，不能出现随机手势回退。
2. OLED 显示待机、倒计时、识别、电脑出拳和结果页面。
3. 开始键、复位键和消抖正常。
4. 石头、剪刀、布各连续测试至少 10 次，并记录不同光照下的准确率。
5. 摄像头失败或画面无有效手部时必须提示重试，不能生成虚假结果。
6. 玩家胜、电脑胜、平局时 LED 和蜂鸣器反馈正确。
7. 连接设备 AP，确认 `/` 和 `/stream` 可访问；进入倒计时和识别阶段时不与摄像头取帧冲突。
8. 连续运行至少 30 分钟，观察是否重启、内存不足或视频流卡死。

## 6. 调参

识别参数集中在 `include/HandRecognition.h`：

- `SKIN_MIN_RATIO` / `SKIN_MAX_RATIO`
- `CONF_THRESHOLD`
- `RECOG_NUM_FRAMES`
- `ROI_PERCENT`

调参应保留真实测试记录，避免只针对单一光线和背景优化。
