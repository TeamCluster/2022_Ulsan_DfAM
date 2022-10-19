## USB (Untouchable Stop Bell)
```비접촉식 버스 하차벨 By Team 나이커```<br />
2022.09.29
***
### **I. 개요**
<img src=image/0.jpg width="60%" height="60%"/><br />
<img src=image/1.jpg width="60%" height="60%"/><br />
**USB** (**Untouchable Stop Bell**) 은 기존 버튼식의 버스 하차벨과 다르게 직접 접촉하지 않고, <u>손을 움직이는 것으로 버스 하차벨을 작동시킬 수 있게</u> 만들어 보자는 아이디어에서 시작되었고, 이와 더불어 블루투스를 통해 통신하는 방식으로 하차벨을 작동시키는 방법도 적용하였습니다.

이 작품은 Team 나이커에 의해 제작되었고, 2022 제6회 울산 DfAM<sup>[1](#foot_1)</sup> 경진대회에 생활부문으로 출품되어 입상하였습니다.<sup>[2](#foot_2)</sup>

해당 작품의 가장 큰 특징들은 다음과 같습니다.

- **상징성 :** 해울이와 같은 지역의 마스코트를 각인하여 기존의 버스 하차벨들과는 디자인적인 면에서 차별성을 지닙니다.
- **감염 예방 :** 많은 접촉이 일어날 수 있는 기존의 버스 하차벨과는 달리 비접촉식이고, 블루투스 통신 방식으로도 작동시킬 수 있어 접촉으로 인한 코로나-19의 n차 감염을 예방할 수 있습니다.
- **단편화 :** 파츠별 모델링을 통해 조립하여 모형을 유동적으로 바꿀 수 있으며, 일부 파츠가 손상되어도 해당 파츠만 출력하여 쉽게 복원이 가능합니다. 
- **편의성 :** 만원 버스일 때나, 교통약자분들의 경우처럼 이동이 불편한 상황에서도, 스마트폰 어플을 활용해 편리하게 하차벨을 원격으로 제어할 수 있습니다.

***
### **II. 작품의 제작 목적/필요성**
<img src=image/2.jpg width="70%" height="70%"/><br />
2019년에 발발하게 된 코로나-19는 비말 감염과 접촉 감염이 모두 일어날 수 있는 바이러스로, 물체에서도 짧게는 몇 시간부터 길게는 며칠까지도 생존할 수 있어 n차 감염을 유발할 수 있음. <br /> <br />
이 때문에 사람들은 접촉에 대한 불안이 증가하였고, 곧 비접촉 기술과 터치리스 기술을 발전시키는 계기가 되었음. 예시로, H사의 에어터치 엘레베이터, I사의 터치리스 변기레버 등 다양한 산업 분야에서도 비접촉 기술을 접목하기 시작하였음.<br /> <br />
다양한 분야에서 활용할 수 있고 접촉을 통한 감염 문제를 해결할 수 있는 ```비접촉 기술```을 기반으로, 만원 버스 등 이동이 불편한 상황에서 손쉽게 버스 하차벨을 작동시킬 수 있는 ```원격 제어 기술```을 더하고, ```디자인```에 차별화를 두어 하차벨을 만들자는 기획을 하게 되었음.


***
### **III. 제작 과정**
<img src=image/3.jpg width="60%" height="60%"/><br />
1. 버스 내부 기둥에 설치할 수 있는 고리형, 벽면에 설치할 수 있는 평면형을 디자인에 맞춰 출력함.
2. <u>조도 센서</u>와 <u>레이저 센서</u>를 활용하여 손의 움직임을 감지하면 LED와 부저가 작동하게 기능을 코딩하여 구현함.
3. 블루투스 제어를 활용하여 스마트폰 어플과 연동하는 기능을 구현함.


***
### **IV. 프로그래밍**
조도 센서/블루투스 기능 작동 코드: USB_CDS.ino
```cpp
// Copyright 2022. Team 나이커 All Rights Reserved.
// ...

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
    // ...
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
            // ...
        }

        // CDS 값 읽어서 출력하기
        // 값 범위는 0~4095로 추정됨
        cds[1] = analogRead(CDS_1_PIN);
        cds[2] = analogRead(CDS_2_PIN);

        // 시리얼 모니터에 조도 센서 값 출력
        Serial.println((String) "CDS_1: " + cds[1] + ", CDS_2: " + cds[2]);

        // 울림 기준치: < 평시의 75% 센서값 (*3/4)
        if ((cds[1] != 0 && cds[1] < std_value[1] * 3 / 4) || (cds[2] != 0 && cds[2] < std_value[2] * 3 / 4)) {
            ring();
        }

        // ...
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


// 연결해둔 LED 채널과 부저 채널이 작동되게 함
void ring() {
    ledcWrite(LED_CHANNEL, LEDC_WRITE_VALUE);
    ledcWriteTone(BUZZER_CHANNEL, RING_TONE);

    delay(RING_TIME);
    
    ledcWrite(LED_CHANNEL, OFF);
    ledcWriteTone(BUZZER_CHANNEL, OFF);
}
```
***
### **V. 우수성 및 기대 효과**
상술한 작품의 특징과 동일합니다.
- 많은 접촉이 일어날 수 있는 기존의 버스 하차벨과는 달리 비접촉식이고, 블루투스 통신 방식으로도 작동시킬 수 있어 접촉으로 인한 코로나-19의 n차 감염을 예방할 수 있습니다.
- 파츠별 모델링을 통해 조립하여 모형을 유동적으로 바꿀 수 있으며, 일부 파츠가 손상되어도 해당 파츠만 출력하여 쉽게 복원이 가능합니다. 
- 만원 버스일 때나, 교통약자분들의 경우처럼 이동이 불편한 상황에서도, 스마트폰 어플을 활용해 편리하게 하차벨을 원격으로 제어할 수 있습니다.

***
### **VI. 덧붙여서**
업로드된 파일들은 비접촉식 버스 하차벨 제작 및 프로그래밍, 기능 구현에 사용된 파일들입니다.

본 파일들의 저작권은 ```Team 나이커```에 귀속되며 <u>어떠한 산업적 목적으로도 2차적으로 재가공/사용될 수 없음</u>을 밝힙니다.

***
Copyright 2022. <u>Team 나이커</u> All Rights Reserved. <br /> <br />

<a name="foot_1">1</a>. Design for Additive Manufacturing Challenge <br />
<a name="foot_2">2</a>. 생활부문 본선 은상(울산대학교)