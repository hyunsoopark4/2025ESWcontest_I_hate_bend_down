#ifndef MPU_H
#define MPU_H

#include <Arduino.h>

// 전역 변수 선언
extern float yaw_initial;

// 함수 선언
void mpu_init();                    // MPU를 사용 가능한 상태로 초기화
void mpu_calibrate_yaw();          // 안정적인 yaw 값을 기다리고 yaw_initial에 저장
float mpu_get_yaw_difference();    // 현재 yaw와 yaw_initial의 차이 계산 (-180 ~ 180)

#endif
