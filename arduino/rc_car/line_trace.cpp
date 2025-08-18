#include <Arduino.h>     // ⚠️ 이 줄이 반드시 필요
#include "line_trace.h"
#include "motor_balance.h"


#define SENSOR_LEFT    7
#define SENSOR_MID_L   8
#define SENSOR_MID_R   9
#define SENSOR_RIGHT   10


void line_trace() {
  int center = digitalRead(SENSOR_MID_R);
  int leftEdge = digitalRead(SENSOR_LEFT);
  int rightEdge = digitalRead(SENSOR_RIGHT);

  // 교차점에서 정지
  if (leftEdge == HIGH || rightEdge == HIGH) {
    balanced_stop();
    Serial.println("== Edge Detected: STOP ==");
    delay(2000);
    return;
  }

  if (center == LOW) {
    balanced_turn_right();
    Serial.println("== 라인 감지 → 방향 A ==");
  } else {
    balanced_turn_left();
    Serial.println("== 라인 미감지 → 방향 B ==");
  }

  delay(1);
}
