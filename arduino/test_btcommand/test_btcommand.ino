#include <Arduino.h>
#include "bt_command.h"
#include "servo_grip.h"
#include "line_trace.h"
#include "dc_motor.h"

// 핀 정의
const int SERVO_PIN = 2;

// 집게 객체 생성
ServoGrip gripper(SERVO_PIN, 80, 15);

void setup()
{
    pinMode(7, INPUT);  // SENSOR_LEFT

    pinMode(9, INPUT);  // SENSOR_MID
    pinMode(10, INPUT); // SENSOR_RIGHT

    Serial.begin(9600);
    Serial.println("Servo Grip Bluetooth Controller Started");

    gripper.attach();
    bt_init();
}

void loop()
{
    int cmd = bt_checkCommand();

    switch (cmd)
    {
    case CMD_OPEN:
        gripper.openGrip();
        break;

    case CMD_CLOSE:
        gripper.closeGrip();
        break;

    case CMD_FORWARD:
        Serial.println("== 전진 명령 수신 ==");
        line_trace();
        break;

    case CMD_FORWARD_TORQUE:
        Serial.println("== 전진 명령 수신 ==");
        line_trace_torque();
        break;

    case CMD_LEFT:
        Serial.println("== 왼쪽 회전 명령 수신 ==");
        turn_left();
        break;

    case CMD_RIGHT:
        Serial.println("== 오른쪽 회전 명령 수신 ==");
        turn_right();
        break;

    case CMD_LEFT_TURBO:
        Serial.println("== 왼쪽 터보 회전 명령 수신 ==");
        turn_left(200, -60);
        break;

    case CMD_RIGHT_TURBO:
        Serial.println("== 오른쪽 터보 회전 명령 수신 ==");
        turn_right(200, -60 );
        break;

    case CMD_BACKWARD:
        Serial.println("== 후진 명령 수신 ==");
        back_on(150);
        delay(500);
        car_stop();
        break;

    case CMD_UNKNOWN:

        // 아무 입력 없으면 무시
        break;
    }
}
