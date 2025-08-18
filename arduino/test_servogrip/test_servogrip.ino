#include <Arduino.h>
#include "bt_command.h"
#include "servo_grip.h"

// 핀 정의
const int SERVO_PIN = 9;

// 집게 객체 생성
ServoGrip gripper(SERVO_PIN, 100, 50);

void setup() {
  Serial.begin(9600);
  Serial.println("Servo Grip Bluetooth Controller Started");

  gripper.attach();
  bt_init();
}

void loop() {
  int cmd = bt_checkCommand();

  switch (cmd) {
    case CMD_OPEN:
      gripper.openGrip();
      break;
    case CMD_CLOSE:
      gripper.closeGrip();
      break;
    case CMD_UNKNOWN:

      // 아무 입력 없으면 무시
      break;
  }
}
