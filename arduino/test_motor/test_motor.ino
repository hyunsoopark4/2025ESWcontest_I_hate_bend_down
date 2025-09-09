#include "dc_motor.h"
#include "servo_grip.h"

// 서보모터 객체 생성 (핀 2번, 열림각도 80도, 닫힘각도 15도)
ServoGrip gripper(2, 80, 15);

void setup() {
    Serial.begin(9600);
    Serial.println("TB6612 Motor Driver + Servo Motor Test Started");
    Serial.println("==============================================");
    
    // 모터 드라이버 초기화
    motor_init();
    Serial.println("Motor driver initialized");
    
    // 서보모터 초기화
    gripper.attach();
    Serial.println("Servo motor initialized");
    
    delay(2000); // 2초 대기
}

void loop() {
    Serial.println("\n=== Test sequence started ===");
    
    // *** 전력 부하 테스트 추가 ***
    powerLoadTest();
    
    Serial.println("Waiting 5 seconds before next cycle...");
    delay(5000);
}

// 전력 부하 테스트 함수
void powerLoadTest() {
    Serial.println("\n=== POWER LOAD TEST ===");
    Serial.println("Testing simultaneous DC motor and servo motor load");
    
    // 테스트 1: DC모터 전진 중 서보 동작
    Serial.println("Test 1: Forward motion + Servo grip operations");
    forward_on(200); // 최대 속도로 전진
    delay(500);
    
    // 서보모터 연속 동작 (부하 증가)
    for (int i = 0; i < 3; i++) {
        Serial.print("  Servo cycle ");
        Serial.println(i + 1);
        gripper.closeGrip(); // 서보 부하
        delay(500);
        gripper.openGrip();  // 서보 부하
        delay(500);
    }
    car_stop();
    delay(1000);
    
    // 테스트 2: DC모터 회전 중 서보 동작
    Serial.println("Test 2: Spin turn + Servo operations");
    spin_right_on(180); // 회전 (높은 토크)
    delay(500);
    
    gripper.closeGrip(); // 동시 부하
    delay(1000);
    gripper.openGrip();
    delay(1000);
    
    car_stop();
    delay(1000);
    
    // 테스트 3: 최대 부하 테스트 (방향 전환 + 서보)
    Serial.println("Test 3: Maximum load test (Direction change + Servo)");
    
    // 급격한 방향 전환과 서보 동작을 동시에
    forward_on(255);   // 최대 속도 전진
    delay(200);
    gripper.closeGrip(); // 서보 동작 시작
    
    back_on(255);      // 급격한 후진 (큰 전류 소모)
    delay(200);
    
    spin_left_on(200); // 좌회전
    delay(200);
    gripper.openGrip(); // 서보 동작
    
    spin_right_on(200); // 우회전
    delay(200);
    
    car_stop();
    delay(1000);
    
    // 테스트 4: 연속 서보 동작 중 모터 동작
    Serial.println("Test 4: Continuous servo motion + Motor operations");
    
    // 서보를 연속으로 움직이면서 DC모터도 동작
    for (int angle = 15; angle <= 80; angle += 5) {
        gripper.moveServoSmoothly(angle);
        
        // 서보 동작 중간에 DC모터 펄스
        if (angle % 20 == 0) {
            forward_on(150);
            delay(100);
            car_stop();
        }
    }
    
    delay(500);
    
    // 테스트 5: 전력 안정성 테스트
    Serial.println("Test 5: Power stability test");
    
    // 10초간 DC모터와 서보를 번갈아 동작
    unsigned long startTime = millis();
    bool motorOn = false;
    bool servoState = false;
    
    while (millis() - startTime < 10000) { // 10초간
        if (millis() % 1000 < 500) { // 0.5초마다 전환
            if (!motorOn) {
                forward_on(180);
                motorOn = true;
            }
        } else {
            if (motorOn) {
                car_stop();
                motorOn = false;
            }
        }
        
        if (millis() % 2000 < 1000) { // 1초마다 서보 전환
            if (!servoState) {
                gripper.closeGrip();
                servoState = true;
            }
        } else {
            if (servoState) {
                gripper.openGrip();
                servoState = false;
            }
        }
        
        delay(50);
    }
    
    car_stop();
    gripper.openGrip(); // 서보를 열린 상태로
    
    Serial.println("=== POWER LOAD TEST COMPLETED ===");
    Serial.println("Monitor for any voltage drops, resets, or erratic behavior");
}