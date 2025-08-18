#include "pid_line.h"
#include "dc_motor.h"

// =================================================================
// PID 상수 (튜닝 가이드)
// 1. 아래 Kp=8.0, Kd=0, Ki=0 으로 시작합니다.
// 2. Kp를 점차 올려 차가 라인을 따라가지만 약간의 "일정한" 떨림이 생길 때까지 조정합니다.
// 3. 최적의 Kp를 찾으면, Kd를 0.5부터 점차 올려 떨림을 잡고 코너링을 부드럽게 만듭니다.
// =================================================================
float Kp = 8.0;         // 비례 상수 (안전한 시작값으로 하향 조정)
float Ki = 0.0;         // 적분 상수 (반드시 0으로 시작)
float Kd = 0.0;         // 미분 상수 (문제를 일으키므로 0에서 다시 시작)

int base_speed = 120;   // 기본 주행 속도
const int LEFT_MOTOR_OFFSET = 20; // 왼쪽 모터 속도 보정값 (필요 시 조정)

// PID 제어 변수
float integral = 0;
float last_error = 0; // 마지막 에러 값 (라인을 놓쳤을 때 사용)

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
    // 1. 센서 값 읽기 (HIGH: 검은선, LOW: 흰바닥)
    bool s_left_end = digitalRead(SENSOR_LEFT);
    bool s_mid_l = digitalRead(SENSOR_MID_L);
    bool s_mid_r = digitalRead(SENSOR_MID_R);
    bool s_right_end = digitalRead(SENSOR_RIGHT);

    // 2. 교차점 감지 (양쪽 끝 센서가 '동시에' 감지될 때만 교차로로 판단)
    if (s_left_end && s_right_end && !crossed) {
        crossed = true;
        node_count++;
        Serial.print("교차점 통과: "); Serial.println(node_count);

        if (node_count >= target_node) {
            car_brake(150);
            return; 
        }
        forward_on(150);
        delay(200);
        integral = 0;
        last_error = 0;
        return;
    }
    if (!s_left_end && !s_right_end) { 
        crossed = false; 
    }

    // 3. PID 에러 계산 (강화된 로직)
    float error;
    if (s_mid_l && s_mid_r) {
        // 최적 상태: 두 중앙 센서 모두 라인 위에 있음
        error = 0;
    } else if (s_mid_l && !s_mid_r) {
        // 오른쪽으로 약간 치우침
        error = 1;
    } else if (!s_mid_l && s_mid_r) {
        // 왼쪽으로 약간 치우침
        error = -1;
    } else if (s_right_end) {
        // '아주 크게' 오른쪽으로 치우침 (강한 제어 개입)
        error = 5; 
    } else if (s_left_end) {
        // '아주 크게' 왼쪽으로 치우침 (강한 제어 개입)
        error = -5;
    } else {
        // 라인을 완전히 놓침
        error = last_error; // 이전 방향 유지
    }

    // 4. PID 제어 계산
    integral += error;
    integral = constrain(integral, -200, 200); // Integral Windup 방지

    float derivative = error - last_error;
    float output = (Kp * error) + (Ki * integral) + (Kd * derivative);

    last_error = error;

    // 5. 모터 속도 제어
    int left_speed = base_speed + output;
    int right_speed = base_speed - output;

    set_motor_speeds(constrain(right_speed, 0, 255), constrain(left_speed + LEFT_MOTOR_OFFSET, 0, 255));

    // 디버깅 출력 (튜닝 시 주석 해제하여 사용)
    /*
    Serial.print("S:");
    Serial.print(s_left_end); Serial.print(s_mid_l); Serial.print(s_mid_r); Serial.print(s_right_end);
    Serial.print(" Err:"); Serial.print(error);
    Serial.print(" Out:"); Serial.print(output);
    Serial.print(" L_Spd:"); Serial.print(left_speed);
    Serial.print(" R_Spd:"); Serial.println(right_speed);
    */
}
