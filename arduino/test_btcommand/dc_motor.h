#ifndef DC_MOTOR_H
#define DC_MOTOR_H
#include <Arduino.h>

/*
 * =================================================
 * === TB6612 모터 드라이버 핀 설정 ===
 * =================================================
 * L: 좌측 모터 (A 채널)
 * R: 우측 모터 (B 채널)
 * IN1, IN2: 방향 제어
 * PWM: 속도 제어 (PWM)
 * STBY: 대기 모드
 */

// --- 대기 모드 핀 ---
#define STBY_PIN 4

// --- 좌측 모터 (A 채널) ---
#define L_IN1_PIN A0
#define L_IN2_PIN A1
#define L_PWM_PIN 5 // PWM

// --- 우측 모터 (B 채널) ---
#define R_IN1_PIN 11
#define R_IN2_PIN 6
#define R_PWM_PIN 3 // PWM


// --- 모터 속도 및 시간 상수 ---
// 속도 관련 상수는 line_trace.h 파일에서 관리합니다.
#include "line_trace.h"

/*
 * =================================================
 * === 함수 원형 ===
 * =================================================
 */

// 모터 핀 초기화
void motor_init();

// 모터 정지
void car_stop();

// 모터 브레이크
void car_brake(int time = 100);

// 전진
void forward_on(int speed = OPT_SPEED);

// 후진
void back_on(int speed = OPT_SPEED);

// 제자리 좌회전
void spin_left_on(int speed = TURN_SPEED);

// 제자리 우회전
void spin_right_on(int speed = TURN_SPEED);

// 좌/우 모터 속도 직접 제어
void set_motor_speeds(int l_speed, int r_speed);

#endif