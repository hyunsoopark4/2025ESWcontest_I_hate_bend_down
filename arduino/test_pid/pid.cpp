#include "pid.h"
#include "dc_motor.h"  // set_motor_speeds 함수 사용

// 전역 PID 객체
LinePID line_pid;

/*
 * =================================================
 * === LinePID 클래스 구현 ===
 * =================================================
 */

// 생성자
LinePID::LinePID() {
    reset();
}

// 초기화 함수 (라인트레이싱 시작 시 호출)
void LinePID::reset() {
    last_error = 0.0;
    integral = 0.0;
    derivative = 0.0;
    last_update_time = millis();
    last_debug_time = 0;
    
    current_state = PID_MANEUVERING;
    state_change_time = millis();
    last_steering_direction = 0.0;
    intersection_detected = false;
    
    sensor_left = false;
    sensor_mid_l = false;
    sensor_mid_r = false;
    sensor_right = false;
    current_sensor_state = 0;
    
    Serial.println("PID Reset - Ready for line tracing");
}

// 센서 노이즈 방지를 위한 신뢰성 있는 센서 읽기
bool LinePID::reliable_sensor_read(int pin) {
    bool first_read = digitalRead(pin);
    bool second_read = digitalRead(pin);
    
    // 두 번의 읽기 결과가 같을 때만 신뢰
    if (first_read == second_read) {
        return first_read;
    }
    
    // 결과가 다르면 한 번 더 읽기
    return digitalRead(pin);
}

// 센서 상태 읽기 함수
int LinePID::read_sensor_state() {
    sensor_left = reliable_sensor_read(SENSOR_LEFT);
    sensor_mid_l = reliable_sensor_read(SENSOR_MID_L);
    sensor_mid_r = reliable_sensor_read(SENSOR_MID_R);
    sensor_right = reliable_sensor_read(SENSOR_RIGHT);
    
    // 센서 조합으로 상태 결정 (향후 확장 가능)
    return (sensor_left << 3) | (sensor_mid_l << 2) | (sensor_mid_r << 1) | sensor_right;
}

// 뒷쪽 센서 기반 교차로 감지 (함수 종료 조건)
bool LinePID::is_rear_intersection() {
    bool left_rear = reliable_sensor_read(7);  // SENSOR_LEFT
    bool right_rear = reliable_sensor_read(10); // SENSOR_RIGHT
    
    // 뒷쪽 센서 중 하나라도 감지되면 교차로
    return (left_rear || right_rear);
}

// 오차 계산 함수 (시간 기반 라인 추적)
float LinePID::calculate_error() {
    current_sensor_state = read_sensor_state();
    unsigned long current_time = millis();
    
    // 교차점 감지 (4개 센서 모두 라인 감지)
    if (sensor_left && sensor_mid_l && sensor_mid_r && sensor_right) {
        if (current_state != PID_INTERSECTION) {
            state_change_time = current_time;
            current_state = PID_INTERSECTION;
        }
        return 0.0; // 교차점에서는 직진
    }
    
    // 라인 감지된 경우 (일반적인 라인 추적)
    if (sensor_left || sensor_mid_l || sensor_mid_r || sensor_right) {
        if (current_state != PID_MANEUVERING) {
            state_change_time = current_time;
            current_state = PID_MANEUVERING;
        }
        
        // 센서 위치 기반 오차 계산 (2.4cm 간격 기준)
        float error = 0.0;
        int sensor_count = 0;
        
        if (sensor_left) {
            error += -3.6; // 가장 왼쪽: -3.6cm
            sensor_count++;
        }
        if (sensor_mid_l) {
            error += -1.2; // 중간 왼쪽: -1.2cm
            sensor_count++;
        }
        if (sensor_mid_r) {
            error += 1.2;  // 중간 오른쪽: +1.2cm
            sensor_count++;
        }
        if (sensor_right) {
            error += 3.6;  // 가장 오른쪽: +3.6cm
            sensor_count++;
        }
        
        last_error = (sensor_count > 0) ? (error / sensor_count) : 0.0;
        return last_error;
    }
    
    // 모든 센서가 라인을 감지하지 못하는 경우 (PID_INLINE)
    if (current_state != PID_INLINE) {
        state_change_time = current_time;
        current_state = PID_INLINE;
    }
    
    // 시간 기반 오차 계산: 오래 감지 안될수록 작은 오차 (정확한 위치)
    unsigned long time_since_state_change = current_time - state_change_time;
    float time_factor = min(time_since_state_change / 1000.0, 2.0); // 최대 2초
    float base_error = 5.0; // 기본 오차값
    
    // 시간이 오래될수록 오차를 줄임 (더 정확하다고 가정)
    float calculated_error = base_error * (1.0 - time_factor * 0.4);
    
    // 마지막으로 감지된 방향을 기억하여 오차 방향 결정
    return (last_error >= 0) ? calculated_error : -calculated_error;
}

// PID 계산 함수
float LinePID::calculate_pid(float error) {
    unsigned long current_time = millis();
    float dt = (current_time - last_update_time) / 1000.0; // 초 단위
    
    if (dt <= 0) dt = 0.02; // 최소 dt 보장 (50Hz 기준)
    
    // 적분 계산 (적분 와인드업 방지)
    integral += error * dt;
    if (integral > 100) integral = 100;
    if (integral < -100) integral = -100;
    
    // 미분 계산
    derivative = (error - last_error) / dt;
    
    // PID 출력 계산
    float output = KP * error + KI * integral + KD * derivative;
    
    // 출력 제한
    if (output > PID_MAX_CORRECTION) output = PID_MAX_CORRECTION;
    if (output < -PID_MAX_CORRECTION) output = -PID_MAX_CORRECTION;
    
    // 변수 업데이트
    last_error = error;
    last_update_time = current_time;
    
    return output;
}

// 모터 제어 적용 함수
void LinePID::apply_motor_control(float correction) {
    int left_speed = PID_BASE_SPEED - correction;
    int right_speed = PID_BASE_SPEED + correction;
    
    // 속도 제한 (0~255)
    left_speed = constrain(left_speed, 0, 255);
    right_speed = constrain(right_speed, 0, 255);
    
    set_motor_speeds(left_speed, right_speed);
}

// 메인 PID 업데이트 함수
void LinePID::update() {
    unsigned long current_time = millis();
    
    // 업데이트 주기 확인 (20ms = 50Hz)
    if (current_time - last_update_time < PID_UPDATE_INTERVAL) {
        return; // 아직 업데이트 시간이 아님
    }
    
    // 오차 계산 (교차점 감지 포함)
    float error = calculate_error();
    
    // 교차점 처리
    if (current_state == PID_INTERSECTION) {
        if (!intersection_detected) {
            // 교차로 진입 시점
            intersection_detected = true;
            Serial.println("PID: Intersection detected - maintaining direction");
        }
        
        // 교차로 중에는 직전 조향 방향 유지
        apply_motor_control(last_steering_direction);
        return;
    }
    else {
        if (intersection_detected) {
            // 교차로 탈출 시점
            intersection_detected = false;
            Serial.println("PID: Intersection passed - resuming PID control");
        }
    }
    
    // 정상 PID 제어
    float output = calculate_pid(error);
    last_steering_direction = output;
    apply_motor_control(output);
    
    // 디버그 출력
    if (current_time - last_debug_time > 500) { // 0.5초마다
        Serial.print("PID Debug - State: ");
        switch (current_state) {
            case PID_MANEUVERING: Serial.print("MANEUVERING"); break;
            case PID_INTERSECTION: Serial.print("INTERSECTION"); break;
            case PID_INLINE: Serial.print("INLINE"); break;
        }
        Serial.print(", Error: "); Serial.print(error);
        Serial.print(", Output: "); Serial.print(output);
        Serial.print(", Sensors: [");
        Serial.print(sensor_left ? "1" : "0"); Serial.print(",");
        Serial.print(sensor_mid_l ? "1" : "0"); Serial.print(",");
        Serial.print(sensor_mid_r ? "1" : "0"); Serial.print(",");
        Serial.print(sensor_right ? "1" : "0"); Serial.println("]");
        last_debug_time = current_time;
    }
    
    last_update_time = current_time;

// 디버깅 정보 출력
void LinePID::print_debug_info() {
    Serial.print("PID State: ");
    switch(current_state) {
        case PID_NORMAL: Serial.print("NORMAL"); break;
        case PID_INTERSECTION: Serial.print("INTERSECTION"); break;
        case PID_LINE_LOST: Serial.print("LINE_LOST"); break;
        case PID_CONTAINMENT: Serial.print("CONTAINMENT"); break;
    }
    Serial.print(" | Error: ");
    Serial.print(last_error);
    Serial.print(" | Integral: ");
    Serial.print(integral);
    Serial.print(" | Steering: ");
    Serial.println(last_steering_direction);
}

/*
 * =================================================
 * === 메인 PID 라인트레이싱 함수 ===
 * =================================================
 */
void pid_line_trace() {
    // PID 시스템 초기화
    line_pid.reset();
    
    Serial.println("Starting PID Line Tracing...");
    
    // 메인 제어 루프
    while (true) {
        // PID 업데이트
        line_pid.update();
        
        // 뒷쪽 센서 기반 교차로 감지 시 함수 종료
        // (SENSOR_LEFT 또는 SENSOR_RIGHT 감지 시)
        if (line_pid.is_rear_intersection()) {
            car_stop();
            Serial.println("PID: Rear intersection detected - function exit");
            return;
        }
        
        // 디버깅 정보 (필요 시 주석 해제)
        // if (millis() % 500 == 0) {
        //     line_pid.print_debug_info();
        // }
        
        // 짧은 딜레이 (CPU 부하 방지)
        delay(1);
    }
}
