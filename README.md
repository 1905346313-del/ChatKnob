# 🎛️ ChatKnob - 旋钮 MQTT 聊天终端

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

ChatKnob 是一个基于 ESP32 开发的硬件聊天终端。它摒弃了传统的实体键盘，利用 **旋转编码器** 和 **三个实体按键** 完成字符输入和发送。所有聊天信息通过 MQTT 协议在局域网/公网中传输，适合学习物联网通信、嵌入式 UI 设计以及自定义聊天硬件。

---

## ✨ 功能特性

- 🔄 **旋转选字**：旋转编码器在 **33~126（`!` ~ `~`）共 94 个 ASCII 可打印字符** 间平滑循环选择
- 🎯 **方向指示**：当前字符旁显示 `↻` / `↺` 方向指示符，上下相邻字符同步预览
- ✅ **三键操作**：
  - `Yes`：将当前字符追加到输入缓冲区末尾
  - `Back`：删除输入缓冲区最后一个字符（退格）
  - `Send`：通过 MQTT 发送完整消息，并清空输入缓冲区
- 🖥️ **OLED 实时显示**：
  - 顶部显示接收到的聊天消息（`ID: message`）
  - 中间显示正在构建的句子
  - 右上角显示当前字符（用 `>` 指示）及上下相邻字符预览
- 🌐 **MQTT 通信**：自动获取 MAC 地址作为唯一设备 ID，支持在线/离线遗嘱消息（LWT）
- 💡 **状态反馈**：按下 `Send` 键发送消息时，板载 LED 灯闪烁提示
- 🔁 **自动重连**：WiFi 超时后自动重启，保证设备长期在线
- 📦 **开源协议**：MIT License，可自由使用、修改、商用

---

## 🛠️ 硬件需求

| 组件 | 型号/规格 | 数量 |
| :--- | :--- | :--- |
| **主控板** | ESP32 开发板（支持 WiFi 和 I2C） | 1 |
| **输入设备** | EC11 旋转编码器（立式直插） | 1 |
| **按键** | 独立微动按键（轻触开关） | 3 |
| **显示** | 0.96 英寸 OLED 屏幕（SSD1306，I2C 接口） | 1 |
| **指示** | 板载 LED（GPIO 2，高电平点亮） | 1 |
| **电阻** | 10kΩ 电阻（可选，用于按键外部上拉） | 3 |

> **注意**：本项目的旋转编码器上的按压开关（SW）未使用，改用三个独立按键分别负责 `Yes`、`Back`、`Send`。

---

## 🔌 引脚接线表

| 硬件组件 | ESP32 引脚 | 说明 |
| :--- | :--- | :--- |
| **编码器 CLK** | GPIO 32 | A 相（正交信号） |
| **编码器 DT** | GPIO 25 | B 相（正交信号） |
| **OLED SDA** | GPIO 5 | I2C 数据线 |
| **OLED SCL** | GPIO 18 | I2C 时钟线 |
| **Yes 按键** | GPIO 23 | 输入（启用内部上拉） |
| **Back 按键** | GPIO 19 | 输入（启用内部上拉） |
| **Send 按键** | GPIO 21 | 输入（启用内部上拉） |
| **LED 指示灯** | GPIO 2 | 高电平点亮（发送时闪烁） |

---

## 📦 依赖库 (Arduino IDE)

在编译之前，请确保安装了以下库：

| 库名称 | 版本 | 用途 |
| :--- | :--- | :--- |
| **[U8g2](https://github.com/olikraus/u8g2)** | ≥ 2.36.19 | OLED 显示驱动 |
| **[PubSubClient](https://github.com/knolleary/pubsubclient)** | ≥ 2.8 | MQTT 通信 |
| **[ArduinoJson](https://github.com/bblanchon/ArduinoJson)** | ≥ 7.0 | JSON 数据解析 |
| **[ESP32Encoder](https://github.com/madhephaestus/ESP32Encoder)** | ≥ 0.12.0 | 旋转编码器计数 |

---

## ⚙️ 配置与烧录

1. 下载本仓库代码，用 Arduino IDE 打开 `chat.ino`。
2. 找到顶部的配置区域，修改以下宏定义：

```cpp
#define SSID      "your_wifi_ssid"       // 你的 WiFi 名称
#define PASSWORD  "your_wifi_password"   // 你的 WiFi 密码
#define CHAT      "your_chat_topic"      // MQTT 主题（建议改为自定义字符串）
#define BROKER    "easyiothings.com"     // MQTT 服务器地址
#define PORT      1883                   // MQTT 端口
