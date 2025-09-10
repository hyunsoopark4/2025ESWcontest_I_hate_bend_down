#include <Arduino.h>
#include "bt_command.h"
#include "servo_grip.h"
#include "line_trace.h"
#include "dc_motor.h"
#include "navigation.h"  // navigation 헤더 추가
// #include "mpu.h"

// 핀 정의
const int SERVO_PIN = 2; // 핀 8은 라인 센서와 충돌하여 2로 변경

// 집게 객체 생성
ServoGrip gripper(SERVO_PIN, 80, 15);

void setup()
{
    Serial.begin(9600);
    Serial.println("--- Setup Start ---");

    Serial.println("Initializing sensors...");
    line_trace_init(); // 라인 트레이서 센서 핀 초기화
    Serial.println("Sensors initialized.");

    Serial.println("Initializing motors...");
    motor_init(); // TB6612 모터 드라이버 초기화
    Serial.println("Motors initialized.");

    Serial.println("Attaching gripper...");
    gripper.attach();
    Serial.println("Gripper attached.");

    Serial.println("Initializing Bluetooth...");
    bt_init();
    Serial.println("Bluetooth initialized.");

    Serial.println("Initializing MPU...");
    // mpu_init();
    Serial.println("MPU initialized.");

    Serial.println("Initializing navigation...");
    init_navigation();
    Serial.println("Navigation initialized.");

    Serial.println("Calibrating MPU yaw...");
    // mpu_calibrate_yaw();
    Serial.println("MPU calibrated.");
    
    Serial.println("--- Setup Complete ---");
}

void loop()
{
    int cmd = bt_checkCommand();

    // --- DEBUGGING --- //
    // 블루투스로 어떤 명령이 수신되는지 시리얼 모니터에 즉시 출력합니다.
    if (cmd != CMD_UNKNOWN) {
        Serial.print("### Command Received: ");
        Serial.println(cmd);
    }
    // --- END DEBUGGING --- //

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
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_FORWARD_TORQUE:
        Serial.println("== 전진 명령 수신 ==");
        line_trace_torque();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_LEFT:
        Serial.println("== 왼쪽 회전 명령 수신 ==");
        // turn_left_stable();
        turn_left();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_RIGHT:
        // 문제가 되는 부분인지 확인하기 위해 추가 디버깅 메시지 출력
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        Serial.println("!!! EXECUTING CMD_RIGHT (회전) !!!");
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        turn_right();
        // turn_right_stable();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_LEFT_TURBO:
        Serial.println("== 왼쪽 터보 회전 명령 수신 ==");
        turn_left();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_RIGHT_TURBO:
        Serial.println("== 오른쪽 터보 회전 명령 수신 ==");
        turn_right();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_BACKWARD:
            Serial.println("== 후진 명령 수신 ==");
            back_on(150);
            delay(500);
            car_stop();
            send_current_state();  // 현재 상태 전송
            break;

        case CMD_FF: // FF 명령어 케이스 추가
            Serial.println("== 짧은 전진 명령 수신 ==");
            forward_on(150);
            delay(500);
            car_stop();
            send_current_state();
            break;

    case CMD_STATUS:
        send_current_state();  // 상태 요청 시 현재 상태 전송
        break;

    case CMD_MOVE_TO:
        Serial.println("== 좌표 이동 명령 수신 ==");
        // move_to_position은 자체적으로 현재 상태를 전송하므로
        // 추가 send_current_state() 호출이 필요 없음
        break;

    case CMD_LEFT_TORQUE:
        Serial.println("== 토크 왼쪽 회전 명령 수신 ==");
        torque_turn_left();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_RIGHT_TORQUE:
        Serial.println("== 토크 오른쪽 회전 명령 수신 ==");
        torque_turn_right();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_FIND_LINE:
        Serial.println("== 라인 찾기 명령 수신 ==");
        find_line();
        send_current_state();  // 현재 상태 전송
        break;

    case CMD_UNKNOWN:
        // 아무 입력 없으면 무시
        break;
    }
}
