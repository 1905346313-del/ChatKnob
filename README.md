# 🎛️ ChatKnob - 旋钮 MQTT 聊天终端

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

ChatKnob 是一个基于 ESP32 开发的硬件聊天终端。它摒弃了传统的实体键盘，利用 **旋转编码器** 和 **三个实体按键** 完成字符输入和发送。所有聊天信息通过 MQTT 协议在局域网/公网中传输，适合学习物联网通信、嵌入式 UI 设计以及自定义聊天硬件。

---

## ✨ 功能特性

- 🔄 **旋转选字**：旋转编码器在 **33~126（`!` ~ `~`）共 94 个 ASCII 可打印字符** 间平滑循环选择
- 🎯 **方向指示**：顶部导航栏显示 `↻` / `↺` 方向指示符，上下相邻字符同步预览，当前字符居中突出
- ✅ **三键操作**：
  - `Yes`：将当前字符追加到输入缓冲区末尾
  - `Back`：删除输入缓冲区最后一个字符（退格）
  - `Send`：通过 MQTT 发送完整消息，并清空输入缓冲区
- 🖥️ **OLED 实时显示**：
  - 顶部导航栏：方向指示符 + 相邻字符 + 当前选中字符（居中）
  - 第二行：接收到的聊天消息（`ID: message`）
  - 第三行：正在构建的输入句子
- 🌐 **MQTT 通信**：自动获取 MAC 地址作为唯一设备 ID，支持在线/离线遗嘱消息（LWT）
- 💡 **状态反馈**：按下 `Send` 键发送消息时，板载 LED 灯闪烁提示
- 🔁 **自动重连**：WiFi 超时后自动重启，保证设备长期在线
- 📦 **开源协议**：MIT License，可自由使用、修改、商用

---

## 🖥️ UI 布局
┌──────────────────────────────────────┐

│ ↻ 上一个 [当前字符] 下一个 ↺ │ Y = 0

│ [接收到的消息] │ Y = 16

│ [正在输入的句子] │ Y = 32

└──────────────────────────────────────┘

text

- 顶部导航栏：当前字符居中，左右显示相邻字符和方向指示符
- 消息接收区：显示最新的聊天消息
- 输入编辑区：显示正在构建的句子，实时更新

---

## 🛠️ 硬件需求

| 组件 | 型号/规格 | 数量 |
| :--- | :--- | :--- |
| **主控板** | ESP32 开发板（支持 WiFi 和 I2C） | 1 |
| **输入设备** | EC11 旋转编码器（立式直插） | 1 |
| **按键** | 独立微动按键（轻触开关） | 3 |
| **显示** | 0.96 英寸 OLED 屏幕（SSD1306，I2C 接口） | 1 |
| **指示** | 板载 LED（GPIO 2，高电平点亮） | 1 |

> **注意**：本项目的旋转编码器上的按压开关（SW）损坏，改用三个独立按键分别负责 `Yes`、`Back`、`Send`。

---

## 🔌 引脚接线表

| 硬件组件 | ESP32 引脚 | 说明 |
| :--- | :--- | :--- |
| **编码器 CLK** | GPIO 32 | A 相 |
| **编码器 DT** | GPIO 25 | B 相 |
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

1. 下载本仓库代码，用 Arduino IDE 打开 `ChatKnob.ino`。
2. 找到顶部的配置区域，修改以下宏定义：

```cpp
#define SSID      "your_wifi_ssid"       // 你的 WiFi 名称
#define PASSWORD  "your_wifi_password"   // 你的 WiFi 密码
#define CHAT      "your_chat_topic"      // MQTT 主题（建议改为自定义字符串）
#define BROKER    "easyiothings.com"     // MQTT 服务器地址
#define PORT      1883                   // MQTT 端口
```

选择开发板：Tools → Board → ESP32 Dev Module。

选择正确端口，点击 Upload 烧录。

---

## 🎮 如何使用

上电启动：设备自动连接 WiFi 和 MQTT Broker，OLED 显示 wifi ok 后进入主界面。

选字符：旋转编码器，顶部导航栏的字符会从 ! 到 ~ 平滑循环，当前字符居中突出。

拼消息：

旋转到目标字符，按下 Yes 键，该字符追加到输入缓冲区（屏幕第三行显示）。

如果输错了，按下 Back 键删除最后一个字符。

发送：按下 Send 键，完整消息通过 MQTT 发布，输入缓冲区自动清空，LED 闪烁提示。

接收消息：其他设备发来的消息会显示在屏幕第二行（格式：发送者ID: 消息内容）。

---

## 📂 代码结构

```text
ChatKnob.ino
├── 配置区 (WiFi / MQTT / 引脚定义 / 常量)
├── setup()
│   ├── encoder_init()      // 编码器初始化
│   ├── oled_init()         // OLED 驱动与字体
│   ├── wifi_init()         // WiFi 连接（失败则重启）
│   ├── Generate_MAC_ID()   // MAC 地址 → 设备 ID
│   ├── mqtt_init()         // MQTT 连接 + 遗嘱 + 订阅
│   └── pin_init()          // 按键与 LED 引脚模式
├── loop()
│   ├── mqttClient.loop()   // MQTT 心跳
│   ├── letter()            // 编码器 → 字符映射（取模循环）
│   ├── 按键检测 (Yes/Back/Send)
│   └── show()              // 统一刷新 OLED
├── send_message()          // 统一输出接口
│   ├── mode=1 → MQTT 发布
│   ├── mode=2 → OLED 打印（主字体）
│   └── mode=3 → OLED 打印（符号字体）
├── mqttCallback()          // 接收消息解析
└── 其他辅助函数
```

---

## 📜 许可证

本项目采用 MIT License，可自由使用、修改、分发，甚至用于商业项目。只需保留原始版权声明即可。

---

## 🤝 贡献
欢迎提交 Issue 或 Pull Request，也欢迎 fork 本仓库打造属于你自己的版本！

---

## 📷 效果展示
![ChatKnob 实物展示](./ChatKnob.jpg)
