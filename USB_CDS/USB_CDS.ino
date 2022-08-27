#define CDS_1_PIN 4
#define CDS_2_PIN 13
#define LED_1_PIN 18
#define LED_2_PIN 19
#define BUZZER_PIN 17
#define BUZZER_CHANNEL 0
#define LED_CHANNEL 1  
#define LIGHT_MAX 4095
#define LIGHT_STD 2000

#define FRAME_TIME 33
#define RING_TIME 1000

#define LEDC_WRITE_VALUE 255
#define TONE_A4 440

#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// 모드 변환용 매크로 선언 부분
// 필요 시 하단의 '#define' 앞에 '//'를 넣고 빼서
// 매크로를 활성화/비활성화시킬 수 있음.
#define LEDC_STYLE
// #define CDS_TEST
// #define BUZZER_TEST
// #define LED_TEST
// #define BLUETOOTH_TEST

int cds[3] = {0, };
BluetoothSerial SerialBT;

void cds_test();
void buzzer_test();
void led_test();
void bluetooth_read();
void ring();


void setup() {
    Serial.begin(115200);
    SerialBT.begin("1"); //Bluetooth device 이름
    Serial.println("The device started, now you can pair it with bluetooth!");    

#ifdef LEDC_STYLE
    ledcSetup(BUZZER_CHANNEL, 5000, 12);
    ledcSetup(LED_CHANNEL, 8000, 8);

    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcAttachPin(LED_1_PIN, LED_CHANNEL);
    ledcAttachPin(LED_2_PIN, LED_CHANNEL);
#else
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_1_PIN, OUTPUT);
    pinMode(LED_2_PIN, OUTPUT);
#endif
}

void loop() {
    // CDS 값 읽어서 출력하기
    // 값 범위는 0~4095로 추정됨
    cds[1] = analogRead(CDS_1_PIN);
    cds[2] = analogRead(CDS_2_PIN);
    Serial.println("============");
    Serial.println((String) "CDS_1: " + cds[1] + ", CDS_2: " + cds[2]);

#ifdef CDS_TEST
    cds_test();
#endif

#ifdef BUZZER_TEST
    buzzer_test();
#endif

#ifdef LED_TEST
    led_test();
#endif

#ifdef BLUETOOTH_TEST
    bluetooth_read();
#endif

    if (cds[1] < LIGHT_STD || cds[2] < LIGHT_STD) {
        ring();
    }
    
    delay(FRAME_TIME);
}

void cds_test() {
    if (cds[1] == LIGHT_MAX || cds[2] == LIGHT_MAX) {
        Serial.println("[CDS TEST] LIGHT DETECTED");
    }
    else {
        Serial.println("[CDS TEST ON]");
    }
}

void buzzer_test() {
#ifdef LEDC_STYLE
    // 부저에 A4음 작성
    ledcWriteTone(BUZZER_CHANNEL, TONE_A4);
    Serial.println("[BUZZER TEST] LEDC - Sending Tone A4...");
#else
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("[BUZZER TEST] DIGITAL - Sending Tone A4...");
#endif
}

void led_test() {
#ifdef LEDC_STYLE
    ledcWrite(LED_CHANNEL, LEDC_WRITE_VALUE);
    Serial.println("[LED TEST] LEDC - Sending LED Value...");
#else
    digitalWrite(LED_1_PIN, HIGH);
    digitalWrite(LED_2_PIN, HIGH);
    Serial.println("[LED TEST] DIGITAL - Sending LED Value...");
#endif
}

void bluetooth_read() {
    if (Serial.available()) {
        SerialBT.write(Serial.read());
    }
    if (SerialBT.available()) {
        Serial.write(SerialBT.read());
    }
}

void ring() {
    ledcWrite(LED_CHANNEL, LEDC_WRITE_VALUE);
    ledcWriteTone(BUZZER_CHANNEL, 880);

    delay(1000);
    
    ledcWrite(LED_CHANNEL, 0);
    ledcWriteTone(BUZZER_CHANNEL, 0);
}
