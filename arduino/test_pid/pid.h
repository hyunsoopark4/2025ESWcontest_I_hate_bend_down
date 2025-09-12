#ifndef PID_H
#define PID_H

#include <Arduino.h>

/*
 * =================================================
 * === PID 라인트레이싱 제어 시스템 ===
 * =================================================
 */

// --- PID 계수 (튜닝 가능) ---
#define KP 0.8    // 비례 게인 (Proportional)
#define KI 0.1    // 적분 게인 (Integral) 
#define KD 0.05   // 미분 게인 (Derivative)

// --- 속도 설정 ---
#define PID_BASE_SPEED 35      // TB6612 + 7V + 1kg 기준 최소 스톨 방지 속도
#define PID_MAX_CORRECTION 25  // 최대 조향 보정값 (±25)
#define PID_UPDATE_INTERVAL 20 // PID 업데이트 주기 (ms) = 50Hz

// --- 센서 관련 ---
#define SENSOR_LEFT 7
#define SENSOR_MID_L 8
#define SENSOR_MID_R 9
#define SENSOR_RIGHT 10

// --- 센서 상태 ---
enum SensorState {
    TILT_LEFT,           // 좌측으로 기울어짐
    TILT_RIGHT,          // 우측으로 기울어짐
    FRONT_INTERSECTION,  // 전면 교차로
    REAR_INTERSECTION,   // 후면 교차로 (종료 조건)
    INLINE              // 라인 중앙 (센서 미감지)
};

/*
 * =================================================
 * === PID 클래스 정의 ===
 * =================================================
 */
class LinePID {
private:
    // PID 변수
    float last_error;
    float integral;
    float derivative;
    unsigned long last_update_time;
    
    // 상태 추적
    SensorState current_state;
    unsigned long state_change_time;
    
    // INLINE 타이머 관리
    unsigned long inline_start_time;    // INLINE 상태 시작 시간
    unsigned long inline_total_time;    // 누적된 INLINE 시간
    unsigned long intersection_start_time; // FRONT_INTERSECTION 시작 시간
    unsigned long intersection_time;    // FRONT_INTERSECTION 지속 시간
    
    // PID 계산 결과 저장
    float correction;                   // 계산된 보정값
    
    // 센서 노이즈 방지
    bool reliable_sensor_read(int pin);
    void read_sensor_state();
    
    // 제어 로직
    float calculate_error_with_time(float inline_time);
    void apply_motor_control(float correction);
    
public:
    // 생성자
    LinePID();
    
    // 초기화
    void reset();
    
    // PID 라인트레이싱 실행
    void pid_linetrace();
    
    // 상태 확인
    SensorState get_state() { return current_state; }
};

// 전역 PID 객체
extern LinePID line_pid;

#endif
