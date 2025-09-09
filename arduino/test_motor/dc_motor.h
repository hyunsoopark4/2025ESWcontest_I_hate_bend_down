#ifndef DC_MOTOR_H
#define DC_MOTOR_H
#include <Arduino.h>

/*
 * =================================================
 * === TB6612 Motor Driver Pin Configuration ===
 * =================================================
 * L: Left Motors (Channel A)
 * R: Right Motors (Channel B)
 * IN1, IN2: Direction Control
 * PWM: Speed Control (PWM)
 * STBY: Standby
 */

// --- Standby Pin ---
#define STBY_PIN 4

// --- Left Motors (Channel A) ---
#define L_IN1_PIN A0
#define L_IN2_PIN A1
#define L_PWM_PIN 5 // PWM

// --- Right Motors (Channel B) ---
#define R_IN1_PIN 11
#define R_IN2_PIN 6
#define R_PWM_PIN 3 // PWM


// --- Motor Speed & Duration Constants ---
#define OPT_SPEED 200         // 기본 주행 속도 (150~255 권장)
#define TURN_SPEED 180        // 회전 시 속도
#define TURN_SPEED_SLOW 100   // 회전 시 저속
#define MOVE_TO_CENTER_DURATION 200 // 교차점 중앙으로 이동하는 시간 (ms) - 튜닝 필요

/*
 * =================================================
 * === Function Prototypes ===
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