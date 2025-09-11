#include "pid.h"
#include "dc_motor.h"

void setup() {
    Serial.begin(115200);
    
    // 센서 핀 초기화
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
    
    // 모터 초기화
    motor_setup();
    
    Serial.println("=== PID Line Tracing Test ===");
    Serial.println("Waiting 3 seconds before starting...");
    delay(3000);
    
    // PID 시스템 시작
    line_pid.reset();
    Serial.println("PID system started!");
}

void loop() {
    // PID 업데이트 (50Hz)
    line_pid.update();
    
    // 뒷쪽 센서로 종료 조건 확인
    if (line_pid.is_rear_intersection()) {
        // 교차로 감지 시 정지
        set_motor_speeds(0, 0);
        Serial.println("Rear intersection detected - STOPPING");
        while(1) {
            delay(100); // 무한 대기
        }
    }
    
    // 다른 작업이 있다면 여기에 추가
    // 하지만 PID는 이미 50Hz로 자동 업데이트됨
}
