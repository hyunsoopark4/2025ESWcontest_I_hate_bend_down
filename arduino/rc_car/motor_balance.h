#ifndef MOTOR_BALANCE_H
#define MOTOR_BALANCE_H

#include "dc_motor.h"  // ⚠️ 이 줄 반드시 추가

// 전체 시스템 보정 계수
const int BASE_SPEED = 100;

// 개별 모터 보정값
const int RIGHT_OFFSET = 10;
const int LEFT_OFFSET  = 0;

inline void balanced_forward() {
  set_motor_speeds(BASE_SPEED + RIGHT_OFFSET, BASE_SPEED + LEFT_OFFSET);
}

inline void balanced_turn_left() {
  set_motor_speeds(BASE_SPEED + RIGHT_OFFSET, BASE_SPEED / 2 + LEFT_OFFSET);
}

inline void balanced_turn_right() {
  set_motor_speeds(BASE_SPEED / 2 + RIGHT_OFFSET, BASE_SPEED + LEFT_OFFSET);
}

inline void balanced_stop() {
  car_stop();
}

#endif
