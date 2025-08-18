#include <Arduino.h>
#include "dc_motor.h"
#include "line_trace.h"

void setup() {
  pinMode(7, INPUT);  // SENSOR_LEFT
  pinMode(8, INPUT);  // SENSOR_MID_L
  pinMode(9, INPUT);  // SENSOR_MID_R
  pinMode(10, INPUT); // SENSOR_RIGHT

  Serial.begin(9600);
  car_stop();
}

void loop() {
  line_trace(0);  // 외부 파일에서 정의한 라인트레이싱 함수 호출
}
