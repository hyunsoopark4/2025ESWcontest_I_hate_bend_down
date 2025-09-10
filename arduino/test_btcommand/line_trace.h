#ifndef LINE_TRACE_H
#define LINE_TRACE_H
#include <Arduino.h>

/*
 * =================================================
 * === 모터 속도 및 시간 상수 (주요 설정) ===
 * =================================================
 */
#define OPT_SPEED 100         // 기본 주행 속도 (PWM: 0~255)
#define TURN_SPEED 90        // 기본 회전 속도 (PWM: 0~255)
#define TURN_SPEED_SLOW 90   // 회전 시 감속 속도
#define MOVE_TO_CENTER_DURATION 200 // 교차점 중앙 이동 시간 (ms)

// --- 라인 트레이싱 및 정렬 속도 ---
#define LINE_TRACE_SLOW_SPEED 50 // 라인을 벗어났을 때 한쪽 바퀴의 속도
#define ALIGN_SPEED 80           // 라인 정렬 시 한쪽 바퀴의 속도

// --- 토크 모드 속도 ---
#define SPEED_TORQUE_FAST 255
#define SPEED_TORQUE_SLOW 70
#define SPEED_TURN_TORQUE_FWD 220
#define SPEED_TURN_TORQUE_BWD -150


// 적외선 센서 핀 (라인을 감지하면 HIGH)
#define SENSOR_LEFT 7
#define SENSOR_MID_L 8
#define SENSOR_MID_R 9
#define SENSOR_RIGHT 10

// 가독성 향상을 위한 정의
#define LINE_DETECTED HIGH

// --- 라인 정렬 함수용 상수 ---
#define ALIGN_LEFT 0
#define ALIGN_RIGHT 1
#define ALIGN_FORWARD 0
#define ALIGN_BACKWARD 1

// 함수 선언
void line_trace();  // 일상 주행
void line_trace_torque(); // 목표물 습득 후 토크감 있는 주행
void blno(int speed, bool apply_brake);
void turn_left();
void turn_right();
void align_on_intersection(); // 교차로 라인 정렬 함수

// line_track의 매개변수에 기본값 지정
void line_track(int speed_fast = OPT_SPEED, int speed_slow = LINE_TRACE_SLOW_SPEED);

// 토크 회전 함수 추가
void torque_turn_left();
void torque_turn_right();

// 외부에서 사용할 수도 있도록 공개
extern int node_count;  
extern bool crossed;


void turn_left_stable();
void turn_right_stable();

void line_trace_init(); // 센서 핀 초기화 함수

#endif