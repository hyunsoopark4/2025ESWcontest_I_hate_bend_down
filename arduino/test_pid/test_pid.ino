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
    Serial.println("Automatic line tracing every 5 seconds");
    
    // PID 시스템 준비
    line_pid.reset();
}

void loop() {
    static unsigned long last_trace_time = 0;
    static int trace_count = 1;
    
    // 센서 상태 출력 (1초마다)
    static unsigned long last_sensor_check = 0;
    if (millis() - last_sensor_check > 1000) {
        print_sensor_status();
        last_sensor_check = millis();
    }
    
    // 5초마다 라인트레이싱 실행
    if (millis() - last_trace_time > 5000) {
        Serial.println("===========================================");
        Serial.print("Starting Line Tracing #");
        Serial.println(trace_count);
        Serial.println("===========================================");
        
        // 라인트레이싱 실행
        line_pid.pid_linetrace();
        
        // 완료 메시지
        Serial.println("===========================================");
        Serial.print("Line Tracing #");
        Serial.print(trace_count);
        Serial.println(" COMPLETED!");
        Serial.println("Waiting 5 seconds for next cycle...");
        Serial.println("===========================================");
        
        last_trace_time = millis();
        trace_count++;
    }
    
    delay(100); // CPU 부하 방지
}

// 센서 상태 출력 함수
void print_sensor_status() {
    bool rear_left = digitalRead(SENSOR_LEFT);
    bool front_left = digitalRead(SENSOR_MID_L);
    bool front_right = digitalRead(SENSOR_MID_R);
    bool rear_right = digitalRead(SENSOR_RIGHT);
    
    Serial.print("Sensors [L7:");
    Serial.print(rear_left ? "1" : "0");
    Serial.print(" FL8:");
    Serial.print(front_left ? "1" : "0");
    Serial.print(" FR9:");
    Serial.print(front_right ? "1" : "0");
    Serial.print(" R10:");
    Serial.print(rear_right ? "1" : "0");
    Serial.print("] → ");
    
    // 상태 분석
    if (rear_left || rear_right) {
        Serial.println("REAR_INTERSECTION");
    } else if (front_left && front_right) {
        Serial.println("FRONT_INTERSECTION");
    } else if (front_left && !front_right) {
        Serial.println("TILT_LEFT");
    } else if (!front_left && front_right) {
        Serial.println("TILT_RIGHT");
    } else {
        Serial.println("INLINE");
    }
}
