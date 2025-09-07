#include <Arduino.h>
#include "bt_command.h"
#include "servo_grip.h"
#include "line_trace.h"
#include "dc_motor.h"
#include "navigation.h"  // navigation 헤더 추가
#include "mpu.h"         // mpu 헤더 추가

// 핀 정의
const int SERVO_PIN = 8;

// 집게 객체 생성
ServoGrip gripper(SERVO_PIN, 80, 15);

void setup()
{
    pinMode(7, INPUT);  // SENSOR_LEFT
    pinMode(9, INPUT);  // SENSOR_MID
    pinMode(10, INPUT); // SENSOR_RIGHT

    Serial.begin(9600);

    gripper.attach();

    Serial.println("== Initializing Bluetooth ==");
    bt_init();
    init_navigation();  // 네비게이션 초기화
    
    // MPU 초기화 및 캘리브레이션
    Serial.println("== Initializing MPU ==");
    mpu_init();
    mpu_calibrate_yaw();

    Serial.println("== System Initialized ==");

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
        Serial.println("== Forward command received ==");
        line_trace();
        send_current_state();  // Send current state
        break;

    case CMD_FORWARD_TORQUE:
        Serial.println("== Forward command received ==");
        line_trace_torque();
        send_current_state();  // Send current state
        break;

    case CMD_LEFT:
        Serial.println("== Left turn command received ==");
        turn_left();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_RIGHT:
        Serial.println("== Right turn command received ==");
        turn_right();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_LEFT_TURBO:
        Serial.println("== Left turbo turn command received ==");
        turn_left(200, -60);
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_RIGHT_TURBO:
        Serial.println("== Right turbo turn command received ==");
        turn_right(200, -60);
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_BACKWARD:
        Serial.println("== Backward command received ==");
        back_on(150);
        delay(500);
        car_stop();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_STATUS:
        send_current_state();  // 상태 요청 시 현재 상태 전송
        break;

    case CMD_MOVE_TO:
        Serial.println("== Move to coordinate command received ==");
        // move_to_position은 자체적으로 현재 상태를 전송하므로
        // 추가 send_current_state() 호출이 필요 없음
        break;

    case CMD_HEAD_NORTH:
        Serial.println("== Head north command received ==");
        line_stabilize(TARGET_NORTH);
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_HEAD_EAST:
        Serial.println("== Head east command received ==");
        line_stabilize(TARGET_EAST);
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_HEAD_SOUTH:
        Serial.println("== Head south command received ==");
        line_stabilize(TARGET_SOUTH);
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_HEAD_WEST:
        Serial.println("== Head west command received ==");
        line_stabilize(TARGET_WEST);
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_UNKNOWN:
        // 아무 입력 없으면 무시
        break;
    }
}
