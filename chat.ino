#include <WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include <string.h>
#include <ArduinoJson.h>
#include <ESP32Encoder.h>
#include <Wire.h>

// WiFi
#define SSID    "your_wifi_ssid"
#define PASSWORD    "your_wifi_password"
WiFiClient espClient;
// MQTT
#define BROKER  "easyiothings.com"
#define PORT    1883
#define CHAT     "your_chat_topic"
#define KEEPALIVE_SEC 15    // 此为实际时间的一半
#define MESSAGE_SIZE 128
PubSubClient mqttClient(espClient);
// 旋转编码器
#define CLK 32  // A相
#define DT  25  // B相
ESP32Encoder encoder;
//OLED屏幕
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 5, 18);
#define font u8g2_font_10x20_tf
// 按键
#define yes 23
#define back 19
#define send 21
// 指示灯
#define light 2

char device_id[24];
char currentLetter = 0;
char will_message[MESSAGE_SIZE];
char receive_message[MESSAGE_SIZE*2];
const int CHAR_START = 33;
const int CHAR_COUNT = 94;

void oled_init();
void wifi_init();
void Generate_MAC_ID();
void send_message(const char * message, uint8_t mode=0, uint8_t x=0, uint8_t y=0);
void mqtt_init();
void mqttCallback(char* topic, uint8_t* payload, unsigned int length);
void encoder_init();
void letter(long currentCount);
void show();
void pin_init();

void setup() {
    encoder_init();
    oled_init();
    wifi_init();
    Generate_MAC_ID();
    mqtt_init();
    send_message("online", 1);
    pin_init();
    will_message[0] = '\0';
}

void loop() {
    mqttClient.loop();
    long currentCount = encoder.getCount();
    letter(currentCount);
    // 选中字母
    if (digitalRead(yes) == LOW) {
        delay(20);
        if (digitalRead(yes) == LOW) {
            while(digitalRead(yes) == LOW);
            int len = strlen(will_message);
            if (len < sizeof(will_message) - 1) {
                will_message[len] = currentLetter;
                will_message[len + 1] = '\0';
            }
        }
    }
    // 删除字母
    else if (digitalRead(back) == LOW) {
        delay(20);
        if (digitalRead(back) == LOW) {
            while(digitalRead(back) == LOW);
            int len = strlen(will_message);
            if (len > 0) {
                will_message[len - 1] = '\0';
            }
        }
    }
    // 发送信息
    else if (digitalRead(send) == LOW) {
        delay(20);
        if (digitalRead(send) == LOW) {
            while(digitalRead(send) == LOW);
            digitalWrite(2, HIGH);
            send_message(will_message, 1);
            will_message[0] = '\0';
        }
    }
    // 发送消息时指示灯会极快闪烁
    else {
        digitalWrite(2, LOW);
    }
    show();
    delay(10);
}

void pin_init() {
    pinMode(yes, INPUT_PULLUP);
    pinMode(back, INPUT_PULLUP);
    pinMode(send, INPUT_PULLUP);
    pinMode(light, OUTPUT);
}

void show() {
    u8g2.clearBuffer();
    char temp[4];
    snprintf(temp, sizeof(temp), "%c", currentLetter);
    send_message(temp, 2, 59, 0);
    if (currentLetter != 33 ) {
        snprintf(temp, sizeof(temp), "↻");
        send_message(temp, 3, 0, 0);
        snprintf(temp, sizeof(temp), "%c", currentLetter-1);
        send_message(temp, 2, 10, 0);
    }
    if (currentLetter != 126) {
        snprintf(temp, sizeof(temp), "↺");
        send_message(temp, 3, 121, 0);
        snprintf(temp, sizeof(temp), "%c", currentLetter+1);
        send_message(temp, 2, 111, 0);
    }
    send_message(receive_message, 2, 0, 16);
    send_message(will_message, 2, 0, 32);
    u8g2.sendBuffer();
}

void letter(long currentCount) {
    int offset = (currentCount / 4) % CHAR_COUNT;
    if (offset < 0) {
        offset += CHAR_COUNT;
    }
    currentLetter = CHAR_START + offset;
}

void oled_init() {
    u8g2.begin();
    u8g2.setBusClock(400000);         // 高速模式
    u8g2.setContrast(255);            // 最高对比度
    u8g2.enableUTF8Print();           // 启用 UTF-8 支持中文
    u8g2.setFont(font);
    u8g2.setFontPosTop();             // 字符定位到顶点
}

void wifi_init() {
    WiFi.begin(SSID, PASSWORD);
    int start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 5000) {
            send_message("wifi timeout", 3, 0, 0);
            ESP.restart();
        }
    }
    if (WiFi.status() == WL_CONNECTED) {
        send_message("wifi ok", 3, 0, 0);
    }
}

void Generate_MAC_ID() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(device_id, sizeof(device_id), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void send_message(const char * message, uint8_t mode, uint8_t x, uint8_t y) {
    if (mode == 1) {
        char payload[MESSAGE_SIZE];
        snprintf(payload, sizeof(payload), "{\"ID\":\"%s\",\"message\":\"%s\"}", device_id, message);
        mqttClient.publish(CHAT, payload);
    }
    else if (mode == 2) {
        u8g2.drawUTF8(x, y, message);
    }
    else if (mode == 3) {
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawUTF8(x, y, message);
        u8g2.setFont(font);
    }
}

void mqtt_init() {
    mqttClient.setServer(BROKER, PORT);
    mqttClient.setKeepAlive(KEEPALIVE_SEC);
    mqttClient.setCallback(mqttCallback);
    char payload[MESSAGE_SIZE];
    snprintf(payload, sizeof(payload), "{\"ID\":\"%s\",\"status\":\"offline\"}", device_id);
    mqttClient.connect(device_id, CHAT, 0, true, payload);
    mqttClient.subscribe(CHAT);
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
    }else {
        return;
    }
}

void encoder_init() {
    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    encoder.attachFullQuad(CLK, DT);
    encoder.setFilter(1023);
    encoder.setCount(128);
}
