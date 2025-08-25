#include <Arduino.h>
#include "dc_motor.h"
#include "line_trace.h"
#include "servo_grip.h"

const int SERVO_PIN = 2;

ServoGrip gripper(SERVO_PIN, 100, 75);

void setup() {
  pinMode(7, INPUT);  // SENSOR_LEFT

  pinMode(9, INPUT);  // SENSOR_MID
  pinMode(10, INPUT); // SENSOR_RIGHT

  gripper.attach();

  Serial.begin(9600);
  car_stop();
}


  //gripper.openGrip();
  //gripper.closeGrip();
void loop() {
  // gripper.openGrip();

  line_trace();  // 외부 파일에서 정의한 라인트레이싱 함수 호출

  // gripper.closeGrip();

}
