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

    // INLINE 타이머 초기화
    inline_start_time = millis();
    inline_total_time = 0;
    intersection_start_time = 0;
    intersection_time = 0;

    // PID 계산 결과 초기화
    correction = 0.0;

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
            // FRONT_INTERSECTION 상태 시작 - intersection 타이머 시작
            intersection_start_time = current_time;

            current_state = FRONT_INTERSECTION;
            state_change_time = current_time;
            Serial.println("FRONT_INTERSECTION");
        }
        return; // FRONT_INTERSECTION 상태에서는 여기서 종료
    }

    // FRONT_INTERSECTION 상태 종료 처리
    if (current_state == FRONT_INTERSECTION) {
        // intersection 타이머 중단 및 시간 계산
        intersection_time = current_time - intersection_start_time;
        Serial.print("FRONT_INTERSECTION ended, duration: ");
        Serial.print(intersection_time);
        Serial.println(" ms");
    }

    if (!front_left && front_right) {
        // 좌측 기울어짐
        if (current_state != TILT_LEFT) {
            // INLINE에서 TILT로 변경 시 실제 INLINE 시간 계산
            if (previous_state == INLINE) {
                inline_total_time = (current_time - inline_start_time - intersection_time);
            }

            // 이전 INLINE 시간을 기반으로 PID 계산
            float inline_time = inline_total_time / 1000.0;
            correction = calculate_error_with_time(inline_time);

            // TILT 상태로 변경 시 타이머 초기화
            inline_total_time = 0;
            intersection_time = 0; // intersection_time도 초기화

            current_state = TILT_LEFT;
            state_change_time = current_time;
            Serial.println("State changed to: TILT_LEFT (timer reset, PID calculated)");
        }
    }
    else if (front_left && !front_right) {
        // 우측 기울어짐
        if (current_state != TILT_RIGHT) {
            // INLINE에서 TILT로 변경 시 실제 INLINE 시간 계산
            if (previous_state == INLINE) {
                inline_total_time = (current_time - inline_start_time - intersection_time);
            }

            // 이전 INLINE 시간을 기반으로 PID 계산
            float inline_time = inline_total_time / 1000.0;
            correction = calculate_error_with_time(inline_time);

            // TILT 상태로 변경 시 타이머 초기화
            inline_total_time = 0;
            intersection_time = 0; // intersection_time도 초기화

            current_state = TILT_RIGHT;
            state_change_time = current_time;
            Serial.println("State changed to: TILT_RIGHT (timer reset, PID calculated)");
        }
    }
    else {
        // 라인 중앙 (센서 미감지)
        if (current_state != INLINE) {
            // INLINE 상태로 변경 시 타이머 시작
            inline_start_time = current_time;

            current_state = INLINE;
            state_change_time = current_time;
            Serial.println("INLINE");
        }
    }
}

// INLINE 시간 기반 PID 계산 (TILT 상태 진입 시에만 호출)
float LinePID::calculate_error_with_time(float inline_time) {
    unsigned long current_time = millis();
    float dt = (current_time - last_update_time) / 1000.0;
    if (dt <= 0) dt = 0.02;

    float error = 0.0;

    // INLINE 시간이 길수록 오차 감소 (더 정확한 위치로 간주)
    float time_factor = min(inline_time, 3.0); // 최대 3초
    float base_error = 1.5;
    error = base_error * exp(-time_factor * 0.5); // 지수적 감소

    // 현재 상태에 따른 방향 결정
    if (current_state == TILT_LEFT) {
        error = -error; // 좌측 기울어짐 - 우측으로 조향
    }
    else if (current_state == TILT_RIGHT) {
        error = error;  // 우측 기울어짐 - 좌측으로 조향
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
void LinePID::apply_motor_control(int base_speed, float correction) {
    // base_speed에 비례한 최대 보정값 계산
    int max_correction = base_speed / 2;
    correction = constrain(correction, -max_correction, max_correction);

    int left_speed = base_speed - correction;
    int right_speed = base_speed + correction;

    // 속도 제한 (0~255)
    left_speed = constrain(left_speed, 0, 255);
    right_speed = constrain(right_speed, 0, 255);

    set_motor_speeds(left_speed, right_speed);
}

// PID 라인트레이싱 실행 함수
void LinePID::pid_linetrace(int base_speed) {
    // PID 시스템 초기화
    reset();
    Serial.println("PID Start");

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
            car_brake(200);
            Serial.println("PID: Rear intersection detected - function exit");
            return;
        }

        // 상태별 제어 로직
        switch (current_state) {
        case TILT_LEFT:
            // INLINE이 될 때까지 우회전
            set_motor_speeds(0, base_speed);
            break;

        case TILT_RIGHT:
            // INLINE이 될 때까지 좌회전
            set_motor_speeds(base_speed, 0);
            break;

        case INLINE:
            // 계산된 PID 값으로 직진
            apply_motor_control(base_speed, correction);
            break;

        case FRONT_INTERSECTION:
            // 모터 제어 변경하지 않음 (현재 상태 유지)
            break;

        default:
            set_motor_speeds(0, 0);
            break;
        }

        delay(1); // CPU 부하 방지
    }
}

// 센서 핀 초기화 함수
void sensor_init() {
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
}

// INLINE 정렬 함수 - INLINE 상태가 지정된 시간만큼 유지되면 정렬 완료
bool LinePID::align_inline(int align_speed, unsigned long timeout_ms) {
    unsigned long start_time = millis();
    unsigned long inline_maintain_start = 0;
    bool is_inline_started = false;

    Serial.println("Align inline start");

    while (millis() - start_time < timeout_ms) {
        read_sensor_state();

        switch (current_state) {
        case TILT_LEFT:
            // INLINE이 될 때까지 우회전
            set_motor_speeds(0, align_speed);
            is_inline_started = false;
            break;

        case TILT_RIGHT:
            // INLINE이 될 때까지 좌회전
            set_motor_speeds(align_speed, 0);
            is_inline_started = false;
            break;

        case INLINE:
            if (!is_inline_started) {
                inline_maintain_start = millis();
                is_inline_started = true;
            }

            // INLINE 상태가 2초간 유지되면 정렬 완료
            if (millis() - inline_maintain_start >= 2000) {
                car_brake(100);
                Serial.println("Align inline complete");
                return true;
            }

            // 정렬 중에는 천천히 직진
            set_motor_speeds(align_speed, align_speed);
            break;

        default:
            // 교차로나 기타 상태에서는 정지
            set_motor_speeds(0, 0);
            is_inline_started = false;
            break;
        }

        delay(10);
    }

    car_brake(100);
    Serial.println("Align inline timeout");
    return false;
}

// 교차로 정렬 함수
bool LinePID::align_intersection(int align_speed) {
    Serial.println("Align intersection start");

    // 1단계: 먼저 INLINE 정렬 수행
    if (!align_inline(align_speed, 5000)) {
        Serial.println("Inline alignment failed");
        return false;
    }

    // 2단계: 정렬 과정에서 앞으로 이동했을 수 있으므로 역방향 PID로 보정
    Serial.println("Reverse correction start");
    reverse_pid_linetrace(align_speed, 1000); // 1초간 역방향으로 보정

    Serial.println("Align intersection complete");
    return true;
}

// 역방향 PID 라인트레이싱 (교차로 정렬용)
void LinePID::reverse_pid_linetrace(int base_speed, unsigned long duration_ms) {
    unsigned long start_time = millis();
    reset(); // PID 시스템 초기화

    while (millis() - start_time < duration_ms) {
        unsigned long current_time = millis();

        // 업데이트 주기 확인
        if (current_time - last_update_time < PID_UPDATE_INTERVAL) {
            delay(1);
            continue;
        }

        read_sensor_state();

        switch (current_state) {
        case TILT_LEFT:
            // 역방향이므로 방향이 반대
            set_motor_speeds(-base_speed, 0);
            break;

        case TILT_RIGHT:
            // 역방향이므로 방향이 반대
            set_motor_speeds(0, -base_speed);
            break;

        case INLINE:
            // 역방향 직진
            set_motor_speeds(-base_speed, -base_speed);
            break;

        default:
            set_motor_speeds(0, 0);
            break;
        }

        delay(1);
    }

    car_brake(100);
    Serial.println("Reverse PID complete");
}
