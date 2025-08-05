#ifndef LINETRACING_H
#define LINETRACING_H

#include <Arduino.h>

// =============================================================
//                  센서 및 PID 설정
// =============================================================

// 센서 핀 정의 (기존 pid_line.h와 동일)
#define SENSOR_LEFT    7  // S1 (가장 왼쪽)
#define SENSOR_MID_L   9  // S2
#define SENSOR_MID_R   10 // S3
#define SENSOR_RIGHT   8  // S4 (가장 오른쪽)

// PID 상수 (실제 주행 환경에 맞게 튜닝이 반드시 필요합니다)
#define KP 25.0f  // 비례 상수: 오차에 비례하여 제어. 클수록 반응이 빨라지지만 너무 크면 진동.
#define KI 0.1f   // 적분 상수: 누적된 오차를 보정. 정상상태 오차를 없애지만 너무 크면 오버슈트 발생.
#define KD 20.0f  // 미분 상수: 오차의 변화율에 반응. 오버슈트를 줄이고 안정성을 높임.

#define BASE_SPEED 110 // 기본 주행 속도

// =============================================================
//                      함수 선언
// =============================================================
void linetrace_setup();
void run_linetracing(int target_node);

extern int node_count; // 교차점 카운트 (rc_car.ino에서 사용)

#endif // LINETRACING_H