#include <Arduino.h>
#include "line_trace.h"
#include "dc_motor.h" // set_motor_speeds 직접 사용

#define SENSOR_LEFT 7
#define SENSOR_MID_L 8
#define SENSOR_MID_R 9
#define SENSOR_RIGHT 10

bool lastState = false;

void line_track(int speed_fast, int speed_slow)
{
    int center = digitalRead(SENSOR_MID_R);

    if (center == LOW)
    {                                             // 라인 위
        set_motor_speeds(speed_fast, speed_slow); // 우측 회전
        if (!lastState)
        {
            lastState = true;
            Serial.println("== 감지: 우회전 ==");
        }
    }
    else
    {                                             // 라인 벗어남
        set_motor_speeds(speed_slow, speed_fast); // 좌측 회전
        if (lastState)
        {
            lastState = false;
            Serial.println("== 미감지: 좌회전 ==");
        }
    }

    delayMicroseconds(500); // 빠른 반응
}

void line_trace()
{
    int center = digitalRead(SENSOR_MID_R);
    bool crossed = false;
    bool stabilizing = false;

    int leftEdge = digitalRead(SENSOR_LEFT);
    int rightEdge = digitalRead(SENSOR_RIGHT);

    while (leftEdge == HIGH || rightEdge == HIGH)
    {
        center = digitalRead(SENSOR_MID_R);
        line_track(SPEED_FAST, SPEED_SLOW);
        delay(1);
        leftEdge = digitalRead(SENSOR_LEFT);
        rightEdge = digitalRead(SENSOR_RIGHT);
    }

    // 처음 라인 찾기
    while (center == HIGH)
    {
        center = digitalRead(SENSOR_MID_R);
        line_track(SPEED_FAST, SPEED_SLOW);
        delay(1);
    }

    // 교차점을 만날 때까지 계속 라인트레이싱
    while (true)
    {
        int leftEdge = digitalRead(SENSOR_LEFT);
        int rightEdge = digitalRead(SENSOR_RIGHT);

        if (leftEdge == HIGH || rightEdge == HIGH)
        {
            for (int i = 0; i < 20; i++)
            {                                           // 약 50ms 동안 추가 라인트레이싱
                line_track(SPEED_SLOW, SPEED_SLOW / 2); // 매우 느린 속도로
                delay(5);
            }
            if (leftEdge != digitalRead(SENSOR_LEFT) && rightEdge != digitalRead(SENSOR_RIGHT))
                continue;

            // 교차점 감지
            crossed = true;

            // 급정지로 위치 안정화
            car_brake(200);
            delay(300); // 안정화를 위한 대기 시간

            // 마지막으로 살짝 더 라인트레이싱
            // for(int i = 0; i < 300; i++) {  // 약 50ms 동안 추가 라인트레이싱
            //     line_track(SPEED_FAST , SPEED_SLOW);  // 매우 느린 속도로
            //     delay(1);
            // }

            // car_stop();
            // delay(100);  // 완전 정지를 위한 대기
            return;
        }

        // 교차점 감지 전까지는 일반 라인트레이싱
        line_track(SPEED_FAST, SPEED_SLOW);
    }
}

void line_trace_torque()
{
    int center = digitalRead(SENSOR_MID_R);
    bool crossed = false;

    while (center == HIGH)
    {
        center = digitalRead(SENSOR_MID_R);

        set_motor_speeds(SPEED_SLOW, SPEED_FAST); // 좌측 회전
        delay(10);
    }

    // 교차점을 만날 때까지 계속 라인트레이싱
    while (true)
    {
        int leftEdge = digitalRead(SENSOR_LEFT);
        int rightEdge = digitalRead(SENSOR_RIGHT);

        // 교차점 감지 시 루프 종료

        if (leftEdge == HIGH || rightEdge == HIGH)
        {
            crossed = true;
            line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW);
            continue;
        }
        else if (crossed)
        {
            car_stop();
            return;
        }

        line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW);
    }
}

void turn_left(int speed_turn_fwd, int speed_turn_bwd)
{
    Serial.println("== 왼쪽 회전 시작 ==");

    // 왼쪽 바퀴는 역방향(속도 낮게), 오른쪽 바퀴는 정방향으로 회전
    int reverse_speed = -70; // 역방향 속도를 직접 지정

    // 1. 왼쪽 센서가 라인을 감지할 때까지 좌회전
    spin_left_on(abs(speed_turn_fwd - 20), abs(speed_turn_fwd));
    while (digitalRead(SENSOR_LEFT) == LOW){
        delay(1);
    }

    Serial.println("== 왼쪽 센서 감지됨, 중앙 센서 대기 ==");

    // 2. 중앙 센서가 라인을 감지할 때까지 계속 회전
    while (digitalRead(SENSOR_MID_R) == LOW){
        delay(1);
    }

    while (digitalRead(SENSOR_MID_R) == HIGH){
        delay(1);
    }

    
    car_brake(100);
    delay(300); // 잠시 정지하여 안정화

    // 4. 라인트레이싱으로 정확한 위치 찾기
    int center = digitalRead(SENSOR_MID_R);
    for(int i=0; i<100; i++){
        center = digitalRead(SENSOR_MID_R);
        line_track(SPEED_FAST, SPEED_SLOW);
        delay(1);
    }


    car_stop();
    Serial.println("== 회전 및 정렬 완료 ==");
}


void turn_right(int speed_turn_fwd, int speed_turn_bwd){
    Serial.println("== 오른쪽 회전 시작 ==");

    // 오른쪽 바퀴는 역방향(속도 낮게), 왼쪽 바퀴는 정방향으로 회전
    int reverse_speed = -70; // 역방향 속도를 직접 지정

    forward_on(150);
    delay(200);

    // 1. 오른쪽 센서 감지될 때까지 우회전
    spin_right_on(abs(speed_turn_fwd), abs(speed_turn_fwd - 20));
    while (digitalRead(SENSOR_RIGHT) == LOW)
    {
        delay(1);
    }

    Serial.println("== 오른쪽 센서 감지됨, 중앙 센서 대기 ==");

    // 2. 중앙 센서 감지될 때까지 계속 회전
    while (digitalRead(SENSOR_MID_R) == LOW)
    {
        delay(1);
    }

    car_brake(100);
    delay(300); // 잠시 정지하여 안정화

    // 4. 라인트레이싱으로 정확한 위치 찾기
    int center = digitalRead(SENSOR_MID_R);
    for(int i=0; i<100; i++){
        center = digitalRead(SENSOR_MID_R);
        line_track(SPEED_SLOW, SPEED_FAST);
        delay(1);
    }
    while(center == HIGH){
        center = digitalRead(SENSOR_MID_R);
        line_track(SPEED_SLOW, SPEED_FAST);
        delay(1);
    }

    car_stop();
    Serial.println("== 회전 및 정렬 완료 ==");
}


void torque_turn_left(int speed_turn_fwd, int speed_turn_bwd)
{
    Serial.println("== 토크 왼쪽 회전 시작 ==");

    // 1. 왼쪽 센서가 라인을 감지할 때까지 좌회전
    while (digitalRead(SENSOR_LEFT) == LOW)
    {
        set_motor_speeds(speed_turn_bwd, speed_turn_fwd);
        delay(3);
    }

    Serial.println("== 왼쪽 센서 감지됨, 중앙 센서 대기 ==");

    // 2. 중앙 센서가 라인을 감지할 때까지 계속 회전
    while (digitalRead(SENSOR_MID_R) == LOW)
    {
        set_motor_speeds(speed_turn_bwd, speed_turn_fwd);
        delay(3);
    }

    car_stop();
    delay(50);
    
    // 3. 뒤로 살짝 이동하며 라인 정렬
    back_on(120);    
    delay(150);
    
    // 4. 라인트레이싱으로 정확한 위치 찾기
    int center = digitalRead(SENSOR_MID_R);
    while(center == HIGH) {
        center = digitalRead(SENSOR_MID_R);
        set_motor_speeds(SPEED_TORQUE_SLOW, SPEED_TORQUE_FAST); 
        delay(3);
    }

    car_stop();
    Serial.println("== 토크 회전 및 정렬 완료 ==");
}

void torque_turn_right(int speed_turn_fwd, int speed_turn_bwd)
{
    Serial.println("== 토크 오른쪽 회전 시작 ==");

    // 1. 오른쪽 센서 감지될 때까지 우회전
    while (digitalRead(SENSOR_RIGHT) == LOW)
    {
        set_motor_speeds(speed_turn_fwd, speed_turn_bwd);
        delay(3);
    }

    Serial.println("== 오른쪽 센서 감지됨, 중앙 센서 대기 ==");

    // 2. 중앙 센서 감지될 때까지 계속 회전
    while (digitalRead(SENSOR_MID_R) == LOW)
    {
        set_motor_speeds(speed_turn_fwd, speed_turn_bwd);
        delay(3);
    }

    car_stop();
    delay(50);
    
    // 3. 뒤로 살짝 이동하며 라인 정렬
    back_on(120);
    delay(150);
    
    // 4. 라인트레이싱으로 정확한 위치 찾기
    int center = digitalRead(SENSOR_MID_R);
    while(center == HIGH) {
        center = digitalRead(SENSOR_MID_R);
        set_motor_speeds(SPEED_TORQUE_SLOW, SPEED_TORQUE_FAST);
        delay(3);
    }

    car_stop();
    Serial.println("== 토크 회전 및 정렬 완료 ==");
}
/*
void turn_left_stable()
    {
        Serial.println("== 안정화 좌회전 시작 ==");

        // 1. 교차점 중앙으로 로봇의 회전축을 이동시킵니다.
        // 이 값은 로봇의 속도와 바퀴 위치에 따라 조절해야 하는 가장 중요한 값입니다.
        set_motor_speeds(100, 100);
        delay(150); // 로봇이 교차로 중앙에 올 때까지의 전진 시간 (ms) - **튜닝 필요**

        // 2. 현재 라인을 완전히 벗어날 때까지 좌회전합니다.
        // (모든 센서가 라인 밖(HIGH)을 가리킬 때까지)
        set_motor_speeds(-120, 120); // 제자리 좌회전
        while (digitalRead(SENSOR_LEFT) == LOW || digitalRead(SENSOR_MID_R) == LOW || digitalRead(SENSOR_RIGHT) == LOW)
        {
            delay(1); // 센서가 모두 라인을 벗어날 때까지 대기
        }
        Serial.println("== 현재 라인 벗어남 ==");

        // 3. 이제 새로운 수직 라인을 중앙 센서가 감지할 때까지 계속 회전합니다.
        while (digitalRead(SENSOR_MID_R) == HIGH)
        {
            delay(1); // 중앙 센서가 라인을 찾을 때까지 대기
        }
        Serial.println("== 새로운 라인 감지 ==");

        // 4. 라인을 감지했으므로 즉시 정지하여 오버슈팅을 최소화합니다.
        car_brake(200); // 모터에 강한 제동
        delay(100);     // 안정화를 위한 잠시 대기

        Serial.println("== 회전 완료 ==");
    }
}

    void turn_right_stable()
    {
        Serial.println("== 안정화 우회전 시작 ==");

        // 1. 교차점 중앙으로 로봇의 회전축을 이동시킵니다.
        set_motor_speeds(100, 100);
        delay(150); // 로봇이 교차로 중앙에 올 때까지의 전진 시간 (ms) - **튜닝 필요**

        // 2. 현재 라인을 완전히 벗어날 때까지 우회전합니다.
        set_motor_speeds(120, -120); // 제자리 우회전
        while (digitalRead(SENSOR_LEFT) == LOW || digitalRead(SENSOR_MID_R) == LOW || digitalRead(SENSOR_RIGHT) == LOW)
        {
            delay(1);
        }
        Serial.println("== 현재 라인 벗어남 ==");

        // 3. 새로운 수직 라인을 중앙 센서가 감지할 때까지 계속 회전합니다.
        while (digitalRead(SENSOR_MID_R) == HIGH)
        {
            delay(1);
        }
        Serial.println("== 새로운 라인 감지 ==");

        // 4. 라인을 감지했으므로 즉시 정지합니다.
        car_brake(200);
        delay(100);

        Serial.println("== 회전 완료 ==");
    }
 */