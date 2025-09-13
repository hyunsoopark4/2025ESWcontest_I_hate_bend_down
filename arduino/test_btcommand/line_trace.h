#ifndef LINE_TRACE_H
#define LINE_TRACE_H
#include <Arduino.h>
#include "pid.h"  // PID_BASE_SPEED 사용을 위해 추가

/*
 * =================================================
 * === 모터 속도 및 시간 상수 (주요 설정) ===
 * =================================================
 */
#define OPT_SPEED 100         // 기본 주행 속도 (PWM: 0~255)
#define MOVE_TO_CENTER_DURATION 200 // 교차점 중앙 이동 시간 (ms)

 // --- 라인 트레이싱 및 정렬 속도 ---
#define ALIGN_SPEED 80           // 라인 정렬 시 한쪽 바퀴의 속도

// --- 토크 모드 속도 ---
#define SPEED_TORQUE_FAST 255
#define SPEED_TORQUE_SLOW 70
#define SPEED_TURN_TORQUE_FWD 220
#define REVERSE_SPEED 80

// 센서 핀과 LINE_DETECTED는 pid.h에서 이미 정의됨
// 가독성 향상을 위한 정의
#define LINE_DETECTED HIGH

// 함수 선언
void line_trace(int base_speed = PID_BASE_SPEED);  // PID 기반 라인 추적 (토크 모드 통합)
void blno(int speed, bool apply_brake);
void turn_left();
void turn_right();
void align_on_intersection(bool back_align = true); // 교차로 라인 정렬 함수

// 토크 모드 전용 함수들 (호환성 유지)
void torque_line_track(int speed_fast, int speed_slow);
void torque_turn_left();
void torque_turn_right();

void find_line(); // 새로운 라인 찾기 함수

// PID에서 센서 초기화를 담당하므로 line_trace_init은 제거됨
// PID에서 기본 라인 추적을 담당하므로 line_track은 제거됨

#endif