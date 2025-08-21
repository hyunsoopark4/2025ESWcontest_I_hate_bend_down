#ifndef MOTOR_BALANCE_H
#define MOTOR_BALANCE_H
  
#include "dc_motor.h"  // ⚠️ 이 줄 반드시 추가

// 전체 시스템 보정 계수
const int BASE_SPEED = 130;

// 개별 모터 보정값
const int RIGHT_OFFSET = 0;
const int LEFT_OFFSET  = 10;
/*
처음엔 RIGHT_OFFSET = 0, LEFT_OFFSET = 0으로 시작

로봇이 한쪽으로 자꾸 쏠리면, 반대쪽에 offset을 +10씩 증가

예: 오른쪽으로 자꾸 돌면 → LEFT_OFFSET = +10 또는 RIGHT_OFFSET = -1

집게로 앞 무게가 쏠릴 때는 전체 BASE_SPEED를 110~130까지 증가시켜야 할 수 있음
*/

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
