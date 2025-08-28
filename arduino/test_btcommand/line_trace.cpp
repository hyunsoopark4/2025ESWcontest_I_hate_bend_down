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
    { // 라인 위
        set_motor_speeds(speed_fast, speed_slow); // 우측 회전
        if (!lastState)
        {
            lastState = true;
            Serial.println("== 감지: 우회전 ==");
        }
    }
    else
    { // 라인 벗어남
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

    while(center == HIGH) {
        center = digitalRead(SENSOR_MID_R);
        line_track(SPEED_FAST, SPEED_SLOW);  // 매개변수 명시적 전달
        delay(10);
    }

    // 교차점을 만날 때까지 계속 라인트레이싱
    while (true)
    {
        int leftEdge = digitalRead(SENSOR_LEFT);
        int rightEdge = digitalRead(SENSOR_RIGHT);
        
        if(leftEdge == HIGH || rightEdge == HIGH){
          crossed = true;
          line_track(SPEED_FAST, SPEED_SLOW);
          continue;
        }
        else if (crossed)
        {
          // 교차점 통과 후 중앙 정렬
          center = digitalRead(SENSOR_MID_R);
          if(center == HIGH) { // 라인을 벗어난 경우
            while(center == HIGH) {
              set_motor_speeds(SPEED_SLOW, SPEED_FAST); // 좌회전으로 라인 찾기
              center = digitalRead(SENSOR_MID_R);
              delay(5);
            }
          }
          
          car_stop();
          delay(100);  // 완전 정지를 위한 대기
          return;
        }
        
        line_track(SPEED_FAST, SPEED_SLOW);
    }
}

void line_trace_torque()
{
    int center = digitalRead(SENSOR_MID_R);
    bool crossed = false;

    while(center == HIGH)
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

        if(leftEdge == HIGH || rightEdge == HIGH){
          crossed = true;
          line_track(255,70);
          continue;
        }
        else if (crossed)
        {
          car_stop();
          return;
        }
        
        line_track(255,70);
    }
}

void turn_left(int speed_turn_fwd, int speed_turn_bwd)
{
    Serial.println("== 왼쪽 회전 시작 ==");

    // 왼쪽 바퀴는 역방향(속도 낮게), 오른쪽 바퀴는 정방향으로 회전
    int reverse_speed = -70;  // 역방향 속도를 직접 지정
    
    // 1. 왼쪽 센서가 라인을 감지할 때까지 좌회전
    while (digitalRead(SENSOR_LEFT) == LOW)
    {
        set_motor_speeds(reverse_speed, speed_turn_fwd);
        delay(5);
    }

    Serial.println("== 왼쪽 센서 감지됨, 중앙 센서 대기 ==");

    // 2. 중앙 센서가 라인을 감지할 때까지 계속 회전
    while (digitalRead(SENSOR_MID_R) == LOW)
    {
        set_motor_speeds(reverse_speed, speed_turn_fwd);
        delay(5);
    }

    car_stop();
    delay(100);  // 잠시 정지하여 안정화
    
    // 3. 뒤로 살짝 이동하며 라인 정렬
    back_on(70);    
    delay(200);  // 200ms 동안 후진
    
    // 4. 라인트레이싱으로 정확한 위치 찾기
    int center = digitalRead(SENSOR_MID_R);
    while(center == HIGH) {
        center = digitalRead(SENSOR_MID_R);
        set_motor_speeds(SPEED_SLOW, SPEED_FAST); 
        delay(5);
    }

    car_stop();
    Serial.println("== 회전 및 정렬 완료 ==");
}

void turn_right(int speed_turn_fwd, int speed_turn_bwd)
{
    Serial.println("== 오른쪽 회전 시작 ==");

    // 오른쪽 바퀴는 역방향(속도 낮게), 왼쪽 바퀴는 정방향으로 회전
    int reverse_speed = -70;  // 역방향 속도를 직접 지정
    
    // 1. 오른쪽 센서 감지될 때까지 우회전
    while (digitalRead(SENSOR_RIGHT) == LOW)
    {
        set_motor_speeds(speed_turn_fwd, reverse_speed);
        delay(5);
    }

    Serial.println("== 오른쪽 센서 감지됨, 중앙 센서 대기 ==");

    // 2. 중앙 센서 감지될 때까지 계속 회전
    while (digitalRead(SENSOR_MID_R) == LOW)
    {
        set_motor_speeds(speed_turn_fwd, reverse_speed);
        delay(5);
    }

    car_stop();
    delay(100);  // 잠시 정지하여 안정화
    
    // 3. 뒤로 살짝 이동하며 라인 정렬
    back_on(70);
    delay(200);  // 200ms 동안 후진
    
    // 4. 라인트레이싱으로 정확한 위치 찾기
    int center = digitalRead(SENSOR_MID_R);
    while(center == HIGH) {
        center = digitalRead(SENSOR_MID_R);
        set_motor_speeds(SPEED_SLOW, SPEED_FAST);
        delay(5);
    }

    car_stop();
    Serial.println("== 회전 및 정렬 완료 ==");
}