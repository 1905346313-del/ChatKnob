# 🎛️ ChatKnob - 旋钮 MQTT 聊天终端

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

ChatKnob 是一个基于 ESP32 开发的硬件聊天终端。它摒弃了传统的实体键盘，利用 **旋转编码器** 和 **三个实体按键** 完成字符输入和发送。所有聊天信息通过 MQTT 协议在局域网/公网中传输，适合学习物联网通信、嵌入式 UI 设计以及自定义聊天硬件。

## ✨ 功能特性
- 🔄 **旋转选字**：旋转编码器从 A-Z 循环选择字母。
- ✅ **三键操作**：`Yes` 追加字母、`Back` 删除字母、`Send` 发送完整句子。
- 🖥️ **OLED 实时显示**：上层显示接收消息，下层显示当前正在输入的内容，右上角显示当前选中字母。
- 🌐 **MQTT 通信**：自动获取 MAC 地址作为唯一身份 ID，支持在线/离线遗嘱消息。
- 💡 **状态反馈**：发送信息时，板载 LED 灯极速闪烁以作提示。

## 🛠️ 硬件需求
- **主控板**：ESP32 开发板 (支持 WiFi 和 I2C)
- **输入设备**：EC11 旋转编码器 × 1
- **按键**：独立微动按键 × 3
- **显示**：0.96寸 OLED 屏幕 (SSD1306, I2C 接口)
- **指示**：板载 LED (GPIO 2)

## 🔌 引脚接线表
| 硬件组件 | ESP32 引脚 | 说明 |
| :--- | :--- | :--- |
| **编码器 CLK** | GPIO 32 | A相 |
| **编码器 DT** | GPIO 25 | B相 |
| **OLED SDA** | GPIO 5 | I2C数据线 |
| **OLED SCL** | GPIO 18 | I2C时钟线 |
| **Yes 按键** | GPIO 23 | 输入（内部上拉） |
| **Back 按键** | GPIO 19 | 输入（内部上拉） |
| **Send 按键** | GPIO 21 | 输入（内部上拉） |
| **LED 指示灯** | GPIO 2 | 高电平点亮 |

## 📦 依赖库 (Arduino IDE)
在编译之前，请确保安装了以下库：
1.  **[U8g2](https://github.com/olikraus/u8g2)** (OLED 显示驱动)
2.  **[PubSubClient](https://github.com/knolleary/pubsubclient)** (MQTT 通信)
3.  **[ArduinoJson](https://github.com/bblanchon/ArduinoJson)** (JSON 数据解析)
4.  **[ESP32Encoder](https://github.com/madhephaestus/ESP32Encoder)** (旋转编码器计数)

## ⚙️ 配置与烧录
1.  打开 `chat.ino` 文件。
2.  找到最上方的 **WiFi** 和 **MQTT** 配置区域。
3.  **替换占位符**为你的真实信息（**注意：切勿将真实密码上传至 GitHub**）：
    ```cpp
    #define SSID    "your_wifi_ssid"
    #define PASSWORD    "your_wifi_password"
    #define CHAT     "your_chat_topic"  // 可以自定义一个 MQTT 主题
