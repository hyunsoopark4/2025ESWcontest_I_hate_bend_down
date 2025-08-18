// 센서 핀 번호 설정 (왼쪽부터)
#define SENSOR_LEFT    7   // 왼쪽 끝
#define SENSOR_MID_L   8   // 가운데 왼쪽 (사용 안 함)
#define SENSOR_MID_R   9   // 가운데 오른쪽 (사용됨)
#define SENSOR_RIGHT   10  // 오른쪽 끝

// 모터 제어 핀은 dc_motor.cpp 기준 유지됨
// R_IA = 6 (PWM), R_IB = 11
// L_IA = 3 (PWM), L_IB = 5

#include <Arduino.h>
#include "dc_motor.h" // 모터 제어 함수들 사용

int speed_low = 50;
int speed_high = 120;

bool lastState = false; // 이전 센서 감지 상태

void setup() {
  pinMode(SENSOR_LEFT, INPUT);
  pinMode(SENSOR_MID_L, INPUT);
  pinMode(SENSOR_MID_R, INPUT);
  pinMode(SENSOR_RIGHT, INPUT);

  Serial.begin(9600);
}

void loop() {
  int leftEdge = digitalRead(SENSOR_LEFT);
  int rightEdge = digitalRead(SENSOR_RIGHT);
  int center = digitalRead(SENSOR_MID_R); // 이 센서로 라인 감지

  // 양 끝 센서 감지되면 정지
  if (leftEdge == HIGH || rightEdge == HIGH) {
    car_stop();
    Serial.println("== Edge Detected: STOP ==");
    delay(2000);
  }

  // 중심 센서 감지 여부에 따라 방향 반전 (흔들기)
  if (center == LOW) {  // 라인 위
    if (!lastState) {
      set_motor_speeds(speed_high, speed_low);  // 우측 바퀴 빠름
      lastState = true;
      Serial.println("== 라인 감지 → 방향 A ==");
    }
  } else {  // 라인 벗어남
    if (lastState) {
      set_motor_speeds(speed_low, speed_high);  // 좌측 바퀴 빠름
      lastState = false;
      Serial.println("== 라인 미감지 → 방향 B ==");
    }
  }
  delay(1);
}
