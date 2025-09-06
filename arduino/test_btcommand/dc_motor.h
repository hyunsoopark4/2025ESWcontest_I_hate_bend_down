#ifndef DC_MOTOR_H
#define DC_MOTOR_H
#include <Arduino.h>

/*
 * =================================================
 * === 4-Wheel Drive Motor Pin Configuration ===
 * =================================================
 * F: Front, R: Rear, L: Left, R: Right
 * IA: Motor Driver Input A
 * IB: Motor Driver Input B
 */

// --- Front Motors (기존 핀) ---
#define R_IA_F 6 // Front-Right Motor IA (PWM)
#define R_IB_F 11
#define L_IA_F 3 // Front-Left Motor IA (PWM)
#define L_IB_F 5

// --- Rear Motors (임시 핀) ---
// !!! 중요: 나중에 실제 연결하실 핀 번호로 반드시 수정해주세요 !!!
#define R_IA_R 12 // Rear-Right Motor IA (임시)
#define R_IB_R 13 // Rear-Right Motor IB (임시)
#define L_IA_R 2  // Rear-Left Motor IA (임시)
#define L_IB_R 4  // Rear-Left Motor IB (임시)


// --- Motor Speed & Duration Constants ---
#define OPT_SPEED 200         // 기본 주행 속도 (150~255 권장)
#define TURN_SPEED 180        // 회전 시 속도
#define TURN_SPEED_SLOW 100   // 회전 시 저속
#define MOVE_TO_CENTER_DURATION 200 // 교차점 중앙으로 이동하는 시간 (ms) - 튜닝 필요

/*
 * =================================================
 * === 4-Wheel Drive Function Prototypes ===
 * =================================================
 * 4륜 구동 로직은 dc_motor.cpp 파일 내에 숨겨져 있습니다.
 * 따라서 함수를 사용하는 쪽에서는 2륜 구동과 동일하게 사용하면 됩니다.
 */

// 모터 정지
void car_stop();

// 모터 브레이크
void car_brake(int time = 100);

// 4륜 전진
void forward_on(int speed = OPT_SPEED);

// 4륜 후진
void back_on(int speed = OPT_SPEED);

// 4륜 제자리 좌회전
void spin_left_on(int speed = TURN_SPEED);

// 4륜 제자리 우회전
void spin_right_on(int speed = TURN_SPEED);

// 4륜 좌/우 모터 속도 직접 제어
// (l_speed: 왼쪽 두 모터 속도, r_speed: 오른쪽 두 모터 속도)
void set_motor_speeds(int l_speed, int r_speed);

#endif