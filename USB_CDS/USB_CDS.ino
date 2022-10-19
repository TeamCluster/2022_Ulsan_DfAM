/*
USB 기능 구현 코드
- 아래 코드는 다음과 같은 기능을 구현한 것입니다.
    - ESP32 보드와 블루투스 연결하여, 어플에서 전송하는 값을 받아 하차벨을 제어함.
    - 조도 센서의 값을 읽어, 일정치 이하로 내려갈 시 (손으로 가릴 시) 하차벨을 제어함.

- 작성자: 김지훈

Copyright 2022. Team 나이커 All Rights Reserved. 
*/


// 상수 선언
#define CDS_1_PIN 4
#define CDS_2_PIN 13
#define LED_1_PIN 18
#define LED_2_PIN 19
#define BUZZER_PIN 17
#define BUZZER_CHANNEL 0
#define LED_CHANNEL 1  
#define LIGHT_MAX 4095
#define LIGHT_STD 2400

#define FRAME_TIME 33
#define RING_TIME 1000
#define OK_TIME 125

#define LEDC_WRITE_VALUE 255
#define RING_TONE 880
#define OK_1_TONE 1175 // D5
#define OK_2_TONE 932 // A#5
#define OFF 0

// 블루투스 헤더 선언
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
#define BLUETOOTH_TEST

// 전역 변수 선언
int cds[3] = {0, };
int total[3] = {0, };
int std_value[3] = {0, };
int cnt = 0;
int ready = 0;
int serialBT_check = 0;
unsigned long setup_finished;
BluetoothSerial SerialBT;

void cds_test();
void buzzer_test();
void led_test();
void bluetooth_read();
void ring();
void ready_ring();
void add_data();

// 초기화 함수
void setup() {
    // 시리얼 모니터 오픈 전 대기용
    delay(5000);
    Serial.begin(115200);

    // 블루투스 시작 확인 코드
    if (!SerialBT.begin("ESP32_USB_1")) {
        Serial.println("An error occurred initializing Bluetooth");
    } 
    else {
        Serial.println("Bluetooth initialized");
    }
    
// LEDC로 작성한 코드를 사용하여, 주파수에 따라 채널을 만들고 부저와 LED 핀을 연결함.
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

    setup_finished = millis();
}

void loop() {
    // 초기 5초는 조도 센서값 데이터 수집에 활용됨.
    if (millis() - setup_finished < 5000) {
        add_data();
    }
    else {
        // 조도 센서값의 평균값 데이터를 얻어온 후 설정함.
        if (!ready) {
            Serial.println((String) "[STD Value] CDS_1: " + std_value[1] + ", CDS_2: " + std_value[2]);
            Serial.println("If value is under 75 percent of STD Value, it rings.");
            Serial.println("");
            Serial.println("");

            ready_ring();
            ready = 1;
        }

        // CDS 값 읽어서 출력하기
        // 값 범위는 0~4095로 추정됨
        cds[1] = analogRead(CDS_1_PIN);
        cds[2] = analogRead(CDS_2_PIN);

        // 시리얼 모니터에 조도 센서 값 출력
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

// 블루투스 신호를 받아와서 하차벨을 제어하는 함수
#ifdef BLUETOOTH_TEST
        bluetooth_read();
#endif

        // 울림 기준치: < 평시의 75% 센서값 (*3/4)
        if ((cds[1] != 0 && cds[1] < std_value[1] * 3 / 4) || (cds[2] != 0 && cds[2] < std_value[2] * 3 / 4)) {
            ring();
        }

        if (!serialBT_check) {
            if (SerialBT.available()) {
                // Serial.println("[BT] Working.");
                serialBT_check = 1;
            }
            else {
                // Serial.println("[BT] Not working.");
            }
        }
    }
    
    delay(FRAME_TIME);
}

// 초기 데이터 수집
// 프레임마다 조도 센서 값을 읽어 평균을 냄
// 데이터 수집 방식을 사용하는 이유: 환경에 따라 초기 조도 센서 값이 달라지는 경우가 있어, 어떤 환경에서도 잘 작동하게 만들기 위함.
void add_data() {
    cnt++;
    total[1] += analogRead(CDS_1_PIN);
    total[2] += analogRead(CDS_2_PIN);
    std_value[1] = total[1] / cnt;
    std_value[2] = total[2] / cnt;
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
    ledcWriteTone(BUZZER_CHANNEL, RING_TONE);
    Serial.println("[BUZZER TEST] LEDC - Sending Tone...");
#else
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("[BUZZER TEST] DIGITAL - Sending Tone...");
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

// 블루투스 값을 읽어와서, 입력이 있으면 하차벨을 작동시킴
void bluetooth_read() {
    if (Serial.available()) {
        SerialBT.write(Serial.read());
    }
    if (SerialBT.available()) {
        char input = SerialBT.read();
        
        Serial.write(SerialBT.read());

        if (input == 'a') {
            ring();
        }
    }
}


// 연결해둔 LED 채널과 부저 채널이 작동되게 함
void ring() {
    ledcWrite(LED_CHANNEL, LEDC_WRITE_VALUE);
    ledcWriteTone(BUZZER_CHANNEL, RING_TONE);

    delay(RING_TIME);
    
    ledcWrite(LED_CHANNEL, OFF);
    ledcWriteTone(BUZZER_CHANNEL, OFF);
}

// 초기 데이터 수집 완료 후 사용 가능한 상태가 됨을 알리는 벨소리
void ready_ring() {
    ledcWriteTone(BUZZER_CHANNEL, OK_1_TONE);
    delay(OK_TIME);

    ledcWriteTone(BUZZER_CHANNEL, OK_2_TONE);
    delay(OK_TIME);

    ledcWriteTone(BUZZER_CHANNEL, OFF);
}