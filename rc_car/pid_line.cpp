#include "pid_line.h"
#include "dc_motor.h"

// PID 상수 (튜닝 필요)
float Kp = 40.0;        // 비례 상수
float Ki = 0.0;         // 적분 상수 (초기 0)
float Kd = 30.0;        // 미분 상수

int base_speed = 120;   // 기본 주행 속도
const int LEFT_MOTOR_OFFSET = 20; // 왼쪽 모터 속도 보정값

// PID 제어 변수
float integral = 0;
float last_error = 0;

// 교차점 감지 변수
int node_count = 0;
bool crossed = false;

void pid_line_setup()
{
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
}

void pid_linetrace(int target_node)
{
    bool s_left_end = digitalRead(SENSOR_LEFT);
    bool s_right_end = digitalRead(SENSOR_RIGHT);
    bool s_mid_l = digitalRead(SENSOR_MID_L);
    bool s_mid_r = digitalRead(SENSOR_MID_R);

    // 1. 교차점 감지 (양쪽 끝 센서)
    if ((s_left_end || s_right_end) && !crossed) {
        crossed = true;
        node_count++;
        Serial.print("교차점 통과: "); Serial.println(node_count);

        if (node_count >= target_node) {
            car_stop();
            return; 
        }
        forward_on(150);
        delay(200);
        // 교차점 통과 후 PID 변수 초기화
        integral = 0;
        last_error = 0;
        return;
    }
    if (!s_left_end && !s_right_end) { crossed = false; }

    // 2. PID 제어 로직 (Straddling 방식)
    float error = 0;
    if (s_mid_l == HIGH && s_mid_r == LOW) {
        // 왼쪽 센서 감지 -> 왼쪽으로 치우침 -> 오른쪽으로 꺾기
        error = -1;
    } else if (s_mid_l == LOW && s_mid_r == HIGH) {
        // 오른쪽 센서 감지 -> 오른쪽으로 치우침 -> 왼쪽으로 꺾기
        error = 1;
    } else {
        // 중앙이거나 라인을 잠시 놓침 -> 직진 또는 이전 상태 유지
        error = 0;
    }

    // 적분항 계산
    integral += error;
    integral = constrain(integral, -100, 100); // Integral Windup 방지

    // 미분항 계산
    float derivative = error - last_error;

    // PID 출력 계산
    float output = (Kp * error) + (Ki * integral) + (Kd * derivative);

    // 이전 에러 값 업데이트
    last_error = error;

    // 3. 모터 속도 제어
    int left_speed = base_speed + output;
    int right_speed = base_speed - output;

    // 모터 속도 제한 및 왼쪽 모터 오프셋 적용
    set_motor_speeds(constrain(right_speed, 0, 255), constrain(left_speed + LEFT_MOTOR_OFFSET, 0, 255));

    // 디버깅 출력
    Serial.print("L:"); Serial.print(s_mid_l);
    Serial.print(" R:"); Serial.print(s_mid_r);
    Serial.print(" Error:"); Serial.print(error);
    Serial.print(" Output:"); Serial.print(output);
    Serial.print(" L_Speed:"); Serial.print(left_speed);
    Serial.print(" R_Speed:"); Serial.println(right_speed);
}
