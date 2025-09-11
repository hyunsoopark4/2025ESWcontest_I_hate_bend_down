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

// --- 제어 상태 ---
enum PIDState {
    PID_MANEUVERING,      // 조향 중 (또는 필요시 PID_TILTLEFT, PID_TILTRIGHT)
    PID_INTERSECTION,     // 교차로 통과 중
    PID_INLINE           // 센서 감지 안된 상태 (라인이 센서 사이에 있음)
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
    unsigned long last_debug_time;
    
    // 상태 추적
    PIDState current_state;
    unsigned long state_change_time;
    float last_steering_direction;
    bool intersection_detected;
    
    // 센서 상태 저장
    bool sensor_left;
    bool sensor_mid_l;
    bool sensor_mid_r;
    bool sensor_right;
    int current_sensor_state;
    
    // 센서 노이즈 방지
    bool reliable_sensor_read(int pin);
    int read_sensor_state();
    
    // 센서 상태 분석
    float calculate_error();
    bool is_rear_intersection();
    
    // 제어 로직
    float calculate_pid(float error);
    void apply_motor_control(float correction);
    
public:
    // 생성자
    LinePID();
    
    // 초기화 (라인트레이싱 함수 호출 시 실행)
    void reset();
    
    // 메인 PID 업데이트 함수
    void update();
    
    // 상태 확인 함수
    PIDState get_state() { return current_state; }
    float get_last_error() { return last_error; }
    
    // 디버깅 함수
    void print_debug_info();
};

// 전역 PID 객체
extern LinePID line_pid;

// 함수 선언
void pid_line_trace();  // PID 기반 라인트레이싱 메인 함수

#endif
