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

// 초기화 함수
void LinePID::reset() {
    last_error = 0.0;
    integral = 0.0;
    derivative = 0.0;
    last_update_time = millis();
    
    current_state = INLINE;
    state_change_time = millis();
    
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

// 센서 상태 읽기 및 현재 상태 결정
void LinePID::read_sensor_state() {
    bool rear_left = reliable_sensor_read(SENSOR_LEFT);   // 핀 7
    bool front_left = reliable_sensor_read(SENSOR_MID_L); // 핀 8
    bool front_right = reliable_sensor_read(SENSOR_MID_R); // 핀 9
    bool rear_right = reliable_sensor_read(SENSOR_RIGHT); // 핀 10
    
    unsigned long current_time = millis();
    SensorState previous_state = current_state;
    
    // 후면 센서 확인 (종료 조건)
    if (rear_left || rear_right) {
        if (current_state != REAR_INTERSECTION) {
            current_state = REAR_INTERSECTION;
            state_change_time = current_time;
            Serial.println("State changed to: REAR_INTERSECTION");
        }
        return;
    }
    
    // 전면 센서 확인
    if (front_left && front_right) {
        // 전면 교차로
        if (current_state != FRONT_INTERSECTION) {
            current_state = FRONT_INTERSECTION;
            state_change_time = current_time;
            Serial.println("State changed to: FRONT_INTERSECTION");
        }
    } else if (front_left && !front_right) {
        // 좌측 기울어짐
        if (current_state != TILT_LEFT) {
            current_state = TILT_LEFT;
            state_change_time = current_time;
            Serial.println("State changed to: TILT_LEFT");
        }
    } else if (!front_left && front_right) {
        // 우측 기울어짐
        if (current_state != TILT_RIGHT) {
            current_state = TILT_RIGHT;
            state_change_time = current_time;
            Serial.println("State changed to: TILT_RIGHT");
        }
    } else {
        // 라인 중앙 (센서 미감지)
        if (current_state != INLINE) {
            current_state = INLINE;
            state_change_time = current_time;
            Serial.println("State changed to: INLINE");
        }
    }
}

// PID 계산 및 모터 제어 (INLINE 상태가 아닐 때만 실행)
float LinePID::calculate_error() {
    unsigned long current_time = millis();
    float dt = (current_time - last_update_time) / 1000.0;
    if (dt <= 0) dt = 0.02;
    
    float error = 0.0;
    
    // 현재 상태에 따른 오차 계산
    switch (current_state) {
        case FRONT_INTERSECTION:
            // 전면 교차로 - 직진
            error = 0.0;
            break;
            
        case TILT_LEFT:
            // 좌측 기울어짐 - 우측으로 조향 필요
            error = -1.2;
            break;
            
        case TILT_RIGHT:
            // 우측 기울어짐 - 좌측으로 조향 필요
            error = 1.2;
            break;
            
        case INLINE:
            // INLINE 상태에서는 이전 INLINE이 아닌 상태의 지속 시간을 기반으로 오차 감소
            unsigned long inline_time = current_time - state_change_time;
            float time_factor = min(inline_time / 1000.0, 3.0); // 최대 3초
            
            // 시간이 길수록 오차 감소 (더 정확한 위치로 간주)
            float base_error = 1.5;
            error = base_error * exp(-time_factor * 0.5); // 지수적 감소
            
            // 마지막 오차 방향 유지
            if (last_error < 0) error = -error;
            break;
            
        default:
            error = 0.0;
            break;
    }
    
    // 적분 계산 (적분 와인드업 방지)
    integral += error * dt;
    integral = constrain(integral, -50, 50);
    
    // 미분 계산
    derivative = (error - last_error) / dt;
    
    // PID 출력 계산
    float output = KP * error + KI * integral + KD * derivative;
    output = constrain(output, -PID_MAX_CORRECTION, PID_MAX_CORRECTION);
    
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

// PID 라인트레이싱 실행 함수
void LinePID::run() {
    // PID 시스템 초기화
    reset();
    Serial.println("Starting PID Line Tracing...");
    
    // 메인 제어 루프
    while (true) {
        unsigned long current_time = millis();
        
        // 업데이트 주기 확인 (20ms = 50Hz)
        if (current_time - last_update_time < PID_UPDATE_INTERVAL) {
            delay(1); // CPU 부하 방지
            continue;
        }
        
        // 센서 상태 읽기 및 상태 결정
        read_sensor_state();
        
        // 후면 교차로 감지 시 정지 및 함수 종료
        if (current_state == REAR_INTERSECTION) {
            set_motor_speeds(0, 0);
            Serial.println("PID: Rear intersection detected - function exit");
            return;
        }
        
        // INLINE 상태가 아닐 때만 PID 계산 및 모터 제어
        if (current_state != INLINE) {
            float correction = calculate_error();
            apply_motor_control(correction);
        }
        // INLINE 상태일 때는 기존 모터 제어를 유지
        
        delay(1); // CPU 부하 방지
    }
}
