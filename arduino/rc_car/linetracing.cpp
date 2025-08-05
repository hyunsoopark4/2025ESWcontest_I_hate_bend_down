#include "linetracing.h"
#include "dc_motor.h"
#include "bt_serial.h"

// PID 제어 관련 전역 변수
float integral = 0;
float last_error = 0;

// 교차점 감지 관련 전역 변수
int node_count = 0;
bool crossed = false;

void linetrace_setup() {
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
}

void run_linetracing(int target_node) {
    // 1. 센서 값 읽기 (검은 라인: 1, 흰 바닥: 0)
    bool s1 = digitalRead(SENSOR_LEFT);
    bool s2 = digitalRead(SENSOR_MID_L);
    bool s3 = digitalRead(SENSOR_MID_R);
    bool s4 = digitalRead(SENSOR_RIGHT);

    // 2. 교차점 감지 (양쪽 끝 센서가 동시에 라인을 감지)
    //    S1과 S4를 교차점 감지 전용으로 사용하면 더 안정적입니다.
    int sensor_sum = s1 + s2 + s3 + s4;
    bool is_cross;

    if (sensor_sum > 2)
        is_cross = true;

    //bool is_cross = s1 && s4;
    if (is_cross && !crossed) {
        crossed = true;
        node_count++;

        if (node_count >= target_node) {
            car_stop();
            Serial.print("Target node reached: ");
            Serial.print(target_node);
            Serial.println(". Stopping.");
            printf_chunked(BTserial, "Target node reached: %d. Stopping.\n", target_node);
            return;
        }

        // 목표에 도달하지 않았으면, 현재 교차점 통과를 로깅
        Serial.print("Cross detected: ");
        Serial.print(node_count);
        Serial.print("/");
        Serial.println(target_node);
        printf_chunked(BTserial, "Cross detected: %d/%d\n", node_count, target_node);
        
        // 교차점을 확실히 지나가기 위해 잠시 직진
        forward_on(180);
        delay(200);

        // 교차점 통과 후 PID 변수 초기화하여 다음 주행에 영향 없도록 함
        integral = 0;
        last_error = 0;
        return; // 이번 사이클은 여기서 종료
    }

    // 교차점이 아닐 때만 crossed 플래그 리셋
    if (!is_cross) {
        crossed = false;
    }

    // 3. 가중 평균을 이용한 에러(Error) 계산
    // 라인의 위치를 -2 ~ +2 사이의 연속적인 값으로 계산합니다.
    // (s1*(-2) + s2*(-1) + s3*1 + s4*2) / (s1+s2+s3+s4)
    // 차가 왼쪽으로 치우치면(s3,s4 감지) error는 양수, 오른쪽으로 치우치면(s1,s2 감지) error는 음수가 됩니다.
    float error = 0;

    if (sensor_sum > 0) {
        error = (float)(s1 * -3 + s2 * -1 + s3 * 1 + s4 * 3) / sensor_sum;
    } else {
        // 라인을 완전히 놓쳤을 경우, 마지막 에러 방향으로 계속 회전
        error = last_error; //(last_error > 0) ? 4 : -4;
    }

    // 4. PID 제어량 계산
    // 비례(Proportional)항
    float p_term = KP * error;

    // 적분(Integral)항 (I-Windup 방지)
    integral += error;
    integral = constrain(integral, -100, 100); // 적분값이 과도하게 커지는 것을 방지
    float i_term = KI * integral;

    // 미분(Derivative)항
    float d_term = KD * (error - last_error);

    // 최종 제어량 (모터 속도 보정값)
    float correction = p_term + i_term + d_term;

    // 다음 계산을 위해 현재 에러 값을 저장
    last_error = error;

    // 5. 모터 속도 제어
    int left_speed = constrain(BASE_SPEED + correction, 50, 200);
    int right_speed = constrain(BASE_SPEED - correction, 50, 200);
    set_motor_speeds(right_speed, left_speed);
}