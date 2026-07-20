# 🎛️ ChatKnob - Rotary Encoder MQTT Chat Terminal

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[中文](./README_zh.md) | [English](./README.md)

ChatKnob is an ESP32-based hardware chat terminal. It replaces traditional keyboards with a **rotary encoder** and **three physical buttons** for character input and sending. All messages are transmitted via MQTT protocol, making it ideal for learning IoT communication, embedded UI design, and custom chat hardware.

---

## ✨ Features

- 🔄 **Rotary character selection**: Smoothly cycle through 94 printable ASCII characters (33–126: `!` ~ `~`)
- 🎯 **Direction indicators**: `↻` / `↺` show rotation direction with adjacent character preview, current character highlighted in the center
- ✅ **Three-button operation**:
  - `YES`: Append current character to input buffer
  - `BACK`: Short press to delete last character (backspace), long press (2s) to enter menu
  - `SEND`: Publish message via MQTT and clear input buffer
- 🖥️ **OLED display**:
  - Top navigation bar: direction indicators + adjacent characters + current character (centered)
  - Second line: Received messages (`ID: message`)
  - Third line: Current input buffer
- 🌐 **MQTT communication**: Auto-generates unique device ID from MAC address, supports Last Will and Testament (LWT)
- 📡 **WiFi provisioning**: First boot automatically creates `ChatKnob_AP` hotspot, OLED displays hotspot name, configure WiFi + MQTT via web portal
- 💾 **Persistent storage**: MQTT parameters (broker, port, topic) saved to Flash, survives power cycle
- 💡 **LED feedback**: Flashes when sending messages
- 🔁 **Auto-reconnect**: Automatically reconnects on WiFi/MQTT disconnection
- 📦 **Open source**: MIT License

---

## 🖥️ UI Layout

```
┌──────────────────────────────────────────────────────────┐
│  ↻   Prev              [Current]               Next  ↺ │  Y = 0
│  [Received message]                                     │  Y = 16
│  [Input buffer]                                         │  Y = 32
└──────────────────────────────────────────────────────────┘
```

- Top navigation bar: Current character centered, adjacent characters with direction indicators
- Message area: Displays latest received message
- Input area: Shows the message being composed, updates in real-time

---

## 🛠️ Hardware Requirements

| Component | Model/Spec | Qty |
| :--- | :--- | :--- |
| **MCU** | ESP32 development board (WiFi + I2C + PCNT support) | 1 |
| **Input** | EC11 rotary encoder (through-hole) | 1 |
| **Buttons** | Tactile push buttons | 3 |
| **Display** | 0.96" OLED screen (SSD1306, I2C) | 1 |
| **Indicator** | Onboard LED (GPIO 2, active high) | 1 |

> **Note**: The push button on the rotary encoder (SW) is not used. Three independent buttons are used instead for `YES`, `BACK`, and `SEND`.

---

## 🔌 Pinout

| Component | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **Encoder CLK** | GPIO 32 | A phase (quadrature signal) |
| **Encoder DT** | GPIO 25 | B phase (quadrature signal) |
| **OLED SDA** | GPIO 5 | I2C data |
| **OLED SCL** | GPIO 18 | I2C clock |
| **YES Button** | GPIO 23 | Input (internal pull-up) |
| **BACK Button** | GPIO 19 | Input (internal pull-up) |
| **SEND Button** | GPIO 21 | Input (internal pull-up) |
| **LED** | GPIO 2 | Active high (flashes on send) |

---

## 📦 Dependencies (Arduino IDE)

| Library | Version | Purpose |
| :--- | :--- | :--- |
| **[U8g2](https://github.com/olikraus/u8g2)** | ≥ 2.36.19 | OLED display driver |
| **[PubSubClient](https://github.com/knolleary/pubsubclient)** | ≥ 2.8 | MQTT communication |
| **[ArduinoJson](https://github.com/bblanchon/ArduinoJson)** | ≥ 7.0 | JSON parsing |
| **[ESP32Encoder](https://github.com/madhephaestus/ESP32Encoder)** | ≥ 0.12.0 | Rotary encoder counting |
| **[WiFiManager](https://github.com/tzapu/WiFiManager)** | ≥ 2.0 | WiFi provisioning |

---

## ⚙️ Build & Flash

1. Clone this repository and open `ChatKnob.ino` with Arduino IDE.
2. Select board: `Tools → Board → ESP32 Dev Module`.
3. Select the correct port and click **Upload**.

> **Note**: WiFi and MQTT credentials are **not hardcoded**. On first boot, the device creates a provisioning hotspot for web-based configuration.

---

## 🎮 How to Use

### First Boot (Provisioning)

1. **Power on**: Device creates `ChatKnob_AP` hotspot, OLED displays the hotspot name.
2. **Connect**: Use your phone or computer to connect to the hotspot (no password).
3. **Configure**: Browser auto-opens the provisioning page. Set:
   - WiFi network and password
   - MQTT broker address
   - MQTT port
   - MQTT topic
4. **Save**: Click save, device reboots and connects to your WiFi and MQTT server.

### Daily Use

1. **Select character**: Rotate the encoder to cycle through characters (`!` ~ `~`), current character highlighted.
2. **Compose message**:
   - Rotate to desired character, press **YES** to append to input buffer (third line).
   - Press **BACK** to delete last character.
3. **Send**: Press **SEND** to publish message via MQTT. Input buffer clears, LED flashes.
4. **Receive**: Messages from other devices appear on the second line (`SenderID: message`).

### Long Press

- **Long press BACK (2 seconds)**: Enter menu (work in progress).

### Reconfigure WiFi or MQTT

Uncomment `wm.resetSettings();` in `setup()`, flash once to clear all saved settings. Device will enter provisioning mode on next boot.

---

## 📂 Code Structure

```
ChatKnob.ino
├── Configuration (WiFi / MQTT / Pin definitions / Constants)
├── setup()
│   ├── wm.resetSettings()   // Uncomment to force reprovisioning
│   ├── pin_init()           // GPIO pin mode setup
│   ├── oled_init()          // OLED driver and font
│   ├── encoder_init()       // Encoder library mounting
│   ├── wifi_init()          // WiFi provisioning via WiFiManager
│   ├── generate_mac_id()    // MAC address → device ID
│   ├── mqtt_init()          // MQTT connection with saved params
│   └── send_message("online", 1) // Online notification
├── loop()
│   ├── mqttClient.loop()    // MQTT keep-alive
│   ├── MQTT reconnect on disconnect
│   ├── letter()             // Encoder → character mapping
│   ├── button()             // Button debounce & dispatch
│   ├── refresh → show()     // Unified OLED refresh
│   └── long_press → menu()  // Menu entry (placeholder)
├── send_message()           // Unified output interface
│   ├── mode=1 → MQTT publish
│   ├── mode=2 → OLED print (primary font)
│   ├── mode=3 → OLED print (symbol font)
│   └── mode=4 → OLED clear + print (emergency)
├── action()                 // Button action executor
├── mqttCallback()           // MQTT message parser
├── save_mqtt_config()       // Save MQTT params to Flash
├── load_mqtt_config()       // Load MQTT params from Flash
└── Helper functions
```

---

## 📷 Gallery

![ChatKnob hardware](./ChatKnob.jpg)

---

## 🤝 Contributing

Issues and Pull Requests are welcome! Feel free to fork and build your own version.

---

## 📜 License

MIT License. Free to use, modify, distribute, even commercially. Retain the original copyright notice.

---

**Enjoy building with ChatKnob! 🎉**