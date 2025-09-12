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
    motor_init();
    
    Serial.println("=== PID Line Tracing Test ===");
    Serial.println("Waiting 3 seconds before starting...");
    delay(3000);
    
    // PID 시스템 시작
    line_pid.reset();
    Serial.println("PID system started!");
}

void loop() {
    // PID 라인트레이싱 실행 (무한 루프 포함)
    line_pid.pid_linetrace();
    
    // pid_linetrace() 함수가 종료되면 (후면 교차로 감지) 무한 대기
    Serial.println("Line tracing completed - infinite wait");
    while(1) {
        delay(1000);
    }
}
