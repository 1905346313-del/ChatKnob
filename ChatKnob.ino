#include <PubSubClient.h>
#include <U8g2lib.h>
#include <string.h>
#include <ArduinoJson.h>
#include <ESP32Encoder.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <Preferences.h>

// WiFi
#define WiFi_name "ChatKnob_AP"     // 可自定义 WiFi 名称
WiFiManager wm;
// MQTT
#define KEEPALIVE_SEC 15    // 心跳间隔（此为实际时间的一半）
#define MESSAGE_SIZE 128    // 限制接受信息和发送信息的大小，接受是发送的2倍
WiFiClient espClient;
PubSubClient mqttClient(espClient);
// 旋转编码器
#define CLK 32  // A相
#define DT  25  // B相
ESP32Encoder encoder;
//OLED屏幕
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 5, 18);
#define FONT u8g2_font_10x20_tf     // oled 屏幕主要使用的字体
bool refresh = true;    // 刷新屏幕标志
// 按键
#define YES 23      // 确认选择字符
#define BACK 19     // 删除最后一个字符
#define SEND 21     // 发送已确认的所有字符
bool long_press = false;    // 长按标志
// 指示灯
#define LIGHT 2     // 板载指示灯引脚，作为发送信息的提示
// 持久化存储
Preferences prefs;
char mqtt_broker[40] = "";   // 存储 mqtt broker 地址
char mqtt_port[6] = "";     // 存储 mqtt broker 端口
char mqtt_topic[40] = "";   // 存储 mqtt topic
// 身份标识
char device_id[24];     // 存储设备 id

char current_letter = 0; // 现在居中的字符
char will_message[MESSAGE_SIZE];    // 将要发送的字符的列表
char receive_message[MESSAGE_SIZE*2];   // 接受到的字符的列表
const int char_start = 33;      // 用于平滑选择字符（起始字符的ascii码）
const int char_count = 94;      // 用于平滑选择字符（字符数量）

void oled_init();
void wifi_init();
void generate_mac_id();
void send_message(const char * message, uint8_t mode=0, uint8_t x=0, uint8_t y=0);
void mqtt_init();
void mqttCallback(char* topic, uint8_t* payload, unsigned int length);
void encoder_init();
void letter(long count);
void show();
void pin_init();
void menu();
void show_menu();
void button();      // 消抖，进入 actionn 函数对应模式
void action(uint8_t mode);    // 执行按键动作
void save_mqtt_config();
void load_mqtt_config();

void setup() {
    pin_init();
    oled_init();
    encoder_init();
    // wm.resetSettings();  // 清除所有 WiFi 信息
    wifi_init();
    generate_mac_id();
    mqtt_init();
    
    send_message("online", 1);
    will_message[0] = '\0';
}

void loop() {
    if (!mqttClient.connected()) {
        wifi_init();
        mqtt_init();
    }
    mqttClient.loop();
    long current_count = encoder.getCount();
    letter(current_count);
    button();
    if (refresh) {
        show();
        refresh = false;
    }
    delay(10);
    if (long_press) {
        menu();
        long_press = false;
        refresh = true;
    }
}

void save_mqtt_config() {
    prefs.begin("mqtt", false);
    prefs.putString("broker", mqtt_broker);
    prefs.putString("port", mqtt_port);
    prefs.putString("topic", mqtt_topic);
    prefs.end();
}

void load_mqtt_config() {
    prefs.begin("mqtt", true);
    strcpy(mqtt_broker, prefs.getString("broker", "").c_str());
    strcpy(mqtt_port, prefs.getString("port", "").c_str());
    strcpy(mqtt_topic, prefs.getString("topic", "").c_str());
    prefs.end();
}

void menu() {
    send_message("menu", 4, 0, 0);
    delay(2000);
    show_menu();
}

void show_menu() {
    u8g2.clearBuffer();
    send_message("", 2, 0, 0);
    u8g2.sendBuffer();
}

void pin_init() {
    // 旋转编码器
    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    // 按键
    pinMode(YES, INPUT_PULLUP);
    pinMode(BACK, INPUT_PULLUP);
    pinMode(SEND, INPUT_PULLUP);
    // 指示灯
    pinMode(LIGHT, OUTPUT);
}

void show() {
    u8g2.clearBuffer();
    char temp[6];
    snprintf(temp, sizeof(temp), "%c", current_letter);
    send_message(temp, 2, 59, 0);
    if (current_letter != 33 ) {
        snprintf(temp, sizeof(temp), "↻");
        send_message(temp, 3, 0, 0);
        snprintf(temp, sizeof(temp), "%c", current_letter-1);
        send_message(temp, 2, 10, 0);
    }
    if (current_letter != 126) {
        snprintf(temp, sizeof(temp), "↺");
        send_message(temp, 3, 121, 0);
        snprintf(temp, sizeof(temp), "%c", current_letter+1);
        send_message(temp, 2, 111, 0);
    }
    send_message(receive_message, 2, 0, 16);
    send_message(will_message, 2, 0, 32);
    u8g2.sendBuffer();
}

void letter(long count) {
    int offset = (count / 4) % char_count;
    if (offset < 0) {
        offset += char_count;
    }
    char new_letter = char_start + offset;
    if (new_letter != current_letter) {
        current_letter = new_letter;
        refresh = true;
    }
}

void oled_init() {
    u8g2.begin();
    u8g2.setBusClock(400000);         // 高速模式
    u8g2.setContrast(255);            // 最高对比度
    u8g2.enableUTF8Print();           // 启用 UTF-8 支持中文
    u8g2.setFont(FONT);
    u8g2.setFontPosTop();             // 字符定位到顶点
}

void wifi_init() {
    send_message(WiFi_name, 4, 0, 0);

    load_mqtt_config();
    WiFiManagerParameter custom_broker("broker", "MQTT 服务器", mqtt_broker, sizeof(mqtt_broker));
    WiFiManagerParameter custom_port("port", "MQTT 端口", mqtt_port, sizeof(mqtt_port));
    WiFiManagerParameter custom_topic("topic", "MQTT 主题", mqtt_topic, sizeof(mqtt_topic));
    wm.addParameter(&custom_broker);
    wm.addParameter(&custom_port);
    wm.addParameter(&custom_topic);

    if (!wm.autoConnect(WiFi_name)) {
        ESP.restart();
    }

    strcpy(mqtt_broker, custom_broker.getValue());
    strcpy(mqtt_port, custom_port.getValue());
    strcpy(mqtt_topic, custom_topic.getValue());
    save_mqtt_config();
}

void generate_mac_id() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(device_id, sizeof(device_id), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void send_message(const char * message, uint8_t mode, uint8_t x, uint8_t y) {
    if (mode == 1) {
        char payload[MESSAGE_SIZE];
        snprintf(payload, sizeof(payload), "{\"ID\":\"%s\",\"message\":\"%s\"}", device_id, message);
        mqttClient.publish(mqtt_topic, payload);
    }
    else if (mode == 2) {
        u8g2.drawUTF8(x, y, message);
    }
    else if (mode == 3) {
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawUTF8(x, y, message);
        u8g2.setFont(FONT);
    }
    else if (mode == 4) {
        u8g2.clearBuffer();
        u8g2.drawUTF8(x, y, message);
        u8g2.sendBuffer();
    }
}

void mqtt_init() {
    if (mqttClient.connected()) {
        return;
    }
    mqttClient.setServer(mqtt_broker, atoi(mqtt_port));
    mqttClient.setKeepAlive(KEEPALIVE_SEC);
    mqttClient.setCallback(mqttCallback);
    int retry = 0;
    while (!mqttClient.connected() && retry < 5) {
        char payload[MESSAGE_SIZE];
        snprintf(payload, sizeof(payload), "{\"ID\":\"%s\",\"status\":\"offline\"}", device_id);
        if (mqttClient.connect(device_id, mqtt_topic, 0, true, payload)) {
            mqttClient.subscribe(mqtt_topic);
            return;
        } else {
            delay(500);
            retry++;
        }
    }
    send_message("restart", 4, 0, 0);
    delay(2000);
    ESP.restart();
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    // 把 payload 转成字符串
    char received[MESSAGE_SIZE*2];
    for (unsigned int i = 0; i < length; i++) {
        received[i] = (char)payload[i];
    }
    received[length] = '\0';
    // 用 JsonDocument 解析，格式: {"ID":"xxxx","message":"hello"}
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, received);
    if (error) {
        return;
    }
    // 提取字段
    const char* sender = doc["ID"];
    const char* msg_text = doc["message"];
    // 忽略自己的消息
    if (sender == nullptr || strcmp(sender, device_id) == 0) {
        return;
    }
    // 在 OLED 上显示
    if (msg_text != nullptr) {
        snprintf(receive_message, sizeof(receive_message), "%s: %s", sender, msg_text);
        refresh = true;
    }else {
        return;
    }
}

void encoder_init() {
    encoder.attachFullQuad(CLK, DT);
    encoder.setFilter(1023);
    encoder.setCount(128);
}

void button() {
    // 选中字符
    if (digitalRead(YES) == LOW) {
        delay(20);
        if (digitalRead(YES) == LOW) {
            while(digitalRead(YES) == LOW);
            action(1);
        }
    }
    // 删除字符 & 长按删除键进入菜单（设置菜单标志）
    else if (digitalRead(BACK) == LOW) {
        delay(20);
        if (digitalRead(BACK) == LOW) {
            long last_time = millis();
            long_press = false;
            while(digitalRead(BACK) == LOW) {
                if (millis() - last_time >= 2000) {
                    long_press = true;
                    break;
                }
            }
            if (!long_press) {
                action(2);
            }
        }
    }
    // 发送信息
    else if (digitalRead(SEND) == LOW) {
        delay(20);
        if (digitalRead(SEND) == LOW) {
            while(digitalRead(SEND) == LOW);
            action(3);
        }
    }
    // 发送消息时指示灯会极快闪烁
    else {
        digitalWrite(LIGHT, LOW);
    }
}

void action(uint8_t mode) {
    if (mode == 1) {
        int len = strlen(will_message);
        if (len < sizeof(will_message) - 1) {
            will_message[len] = current_letter;
            will_message[len + 1] = '\0';
        }
        refresh = true;
    }
    else if (mode == 2) {
        int len = strlen(will_message);
        if (len > 0) {
            will_message[len - 1] = '\0';
        }
        refresh = true;
    }
    else if (mode == 3) {
        send_message(will_message, 1);
        will_message[0] = '\0';
        digitalWrite(LIGHT, HIGH);
        refresh = true;
    }
}
