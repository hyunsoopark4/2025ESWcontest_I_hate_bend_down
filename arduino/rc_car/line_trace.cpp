#include <Arduino.h>
#include "line_trace.h"
#include "dc_motor.h"

// 센서 핀 번호 (왼쪽부터)
#define SENSOR_LEFT    7
#define SENSOR_MID_L   8
#define SENSOR_MID_R   9
#define SENSOR_RIGHT   10

// 속도 설정
const int speed_low = 50;
const int speed_high = 120;

bool lastState = false; // 흔들기용
int node_count = 0;     // 지난 교차점 수
bool crossed = false;   // 현재 교차점 감지 중인지

void line_trace(int target_count) {
  int leftEdge = digitalRead(SENSOR_LEFT);
  int rightEdge = digitalRead(SENSOR_RIGHT);
  int center = digitalRead(SENSOR_MID_R);

  // ✅ 교차점 감지 및 카운트
  if ((leftEdge == HIGH || rightEdge == HIGH) && !crossed) {
    crossed = true;
    node_count++;
    Serial.print("== 교차점 감지: ");
    Serial.println(node_count);

    if (target_count == 0 || node_count >= target_count) {
      // 🎯 마지막 교차점 → 역회전으로 감속 정지
      set_motor_speeds(60, 60);  // 살짝 밀기
      delay(200);

      back_on(70);  // 역회전
      delay(120);

      car_stop();
      Serial.println("== 목표 교차점 도달: 정지 ==");
    } else {
      // 🚗 교차점 밀고 지나가기 → 중복 감지 방지
      set_motor_speeds(speed_high, speed_high);
      delay(200);
    }
  }

  // ✅ 교차점 영역 벗어남
  if (leftEdge == LOW && rightEdge == LOW) {
    crossed = false;
  }

  // ✅ 흔들기 라인트레이싱 로직
  if (center == LOW) {
    if (!lastState) {
      set_motor_speeds(speed_high, speed_low);
      lastState = true;
      Serial.println("== 라인 감지 → 방향 A ==");
    }
  } else {
    if (lastState) {
      set_motor_speeds(speed_low, speed_high);
      lastState = false;
      Serial.println("== 라인 미감지 → 방향 B ==");
    }
  }

  delay(1);
}
