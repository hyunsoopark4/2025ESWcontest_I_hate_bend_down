#include "dc_motor.h"

// 모터 핀 초기화 함수
void motor_init() {
    // 모든 모터 제어 핀을 출력으로 설정
    pinMode(L_IN1_PIN, OUTPUT);
    pinMode(L_IN2_PIN, OUTPUT);
    pinMode(L_PWM_PIN, OUTPUT);
    pinMode(R_IN1_PIN, OUTPUT);
    pinMode(R_IN2_PIN, OUTPUT);
    pinMode(R_PWM_PIN, OUTPUT);
    pinMode(STBY_PIN, OUTPUT);

    // TB6612 드라이버 활성화
    digitalWrite(STBY_PIN, HIGH);
}

// 좌/우 모터 속도 직접 제어 함수
void set_motor_speeds(int l_speed, int r_speed) {
    // 속도 값의 범위를 0-255로 제한
    l_speed = constrain(l_speed, -255, 255);
    r_speed = constrain(r_speed, -255, 255);

    // --- 좌측 모터 제어 ---
    if (l_speed > 0) { // 전진
        digitalWrite(L_IN1_PIN, HIGH);
        digitalWrite(L_IN2_PIN, LOW);
        analogWrite(L_PWM_PIN, l_speed);
    } else if (l_speed < 0) { // 후진
        digitalWrite(L_IN1_PIN, LOW);
        digitalWrite(L_IN2_PIN, HIGH);
        analogWrite(L_PWM_PIN, -l_speed);
    } else { // 정지
        digitalWrite(L_IN1_PIN, LOW);
        digitalWrite(L_IN2_PIN, LOW);
        analogWrite(L_PWM_PIN, 0);
    }

    // --- 우측 모터 제어 ---
    if (r_speed > 0) { // 전진
        digitalWrite(R_IN1_PIN, HIGH);
        digitalWrite(R_IN2_PIN, LOW);
        analogWrite(R_PWM_PIN, r_speed);
    } else if (r_speed < 0) { // 후진
        digitalWrite(R_IN1_PIN, LOW);
        digitalWrite(R_IN2_PIN, HIGH);
        analogWrite(R_PWM_PIN, -r_speed);
    } else { // 정지
        digitalWrite(R_IN1_PIN, LOW);
        digitalWrite(R_IN2_PIN, LOW);
        analogWrite(R_PWM_PIN, 0);
    }
}

// 전진
void forward_on(int speed) {
    set_motor_speeds(speed, speed);
}

// 후진
void back_on(int speed) {
    set_motor_speeds(-speed, -speed);
}

// 제자리 좌회전
void spin_left_on(int speed) {
    set_motor_speeds(speed, -speed);
}

// 제자리 우회전
void spin_right_on(int speed) {
    set_motor_speeds(-speed, speed);
}

// 모터 정지 (관성 주행)
void car_stop() {
    set_motor_speeds(0, 0);
}

// 모터 브레이크 (급정지)
void car_brake(int time) {
    // 양쪽 모터의 IN1, IN2를 모두 HIGH로 설정하여 브레이크 모드 진입
    digitalWrite(L_IN1_PIN, HIGH);
    digitalWrite(L_IN2_PIN, HIGH);
    digitalWrite(R_IN1_PIN, HIGH);
    digitalWrite(R_IN2_PIN, HIGH);
    // PWM은 0으로 설정
    analogWrite(L_PWM_PIN, 0);
    analogWrite(R_PWM_PIN, 0);
    
    delay(time); // 지정된 시간만큼 브레이크 상태 유지
    car_stop();  // 이후 모터 정지
}