#include <Arduino.h>
#include "line_trace.h"
#include "dc_motor.h" // set_motor_speeds 직접 사용

#define SENSOR_LEFT 7
#define SENSOR_MID_L 8
#define SENSOR_MID_R 9
#define SENSOR_RIGHT 10

const int speed_fast = 170;
const int speed_slow = 30;

bool lastState = false;

void line_trace()
{
    int center = digitalRead(SENSOR_MID_R);

    while(center == HIGH)
    {
        center = digitalRead(SENSOR_MID_R);

        set_motor_speeds(speed_slow, speed_fast); // 좌측 회전
        delay(10);
    }

    // 교차점을 만날 때까지 계속 라인트레이싱
    while (true)
    {
        center = digitalRead(SENSOR_MID_R);

        int leftEdge = digitalRead(SENSOR_LEFT);
        int rightEdge = digitalRead(SENSOR_RIGHT);
        
        // 교차점 감지 시 루프 종료
        if (leftEdge == HIGH || rightEdge == HIGH) {
            // 교차점 처리
            forward_on(100);
            delay(100);
            car_stop();
            delay(500);

            break; // while 루프 종료
        }

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
}


void line_trace_torque()
{

    int center = digitalRead(SENSOR_MID_R);
    int leftEdge = digitalRead(SENSOR_LEFT);
    int rightEdge = digitalRead(SENSOR_RIGHT);

    // 교차점에서 정지 or 카운트
    if (leftEdge == HIGH || rightEdge == HIGH)
    {
        // car_brake(100);
        forward_on(255);
        delay(200);
        car_stop();
        delay(2000);
        return;
    }

    if (center == LOW)
    { // 라인 위
        if (!lastState)
        {
            set_motor_speeds(speed_fast, speed_slow); // 우측 회전
            lastState = true;
            Serial.println("== 감지: 우회전 ==");
        }
    }
    else
    { // 라인 벗어남
        if (lastState)
        {
            set_motor_speeds(speed_slow, speed_fast); // 좌측 회전
            lastState = false;
            Serial.println("== 미감지: 좌회전 ==");
        }
    }

    delayMicroseconds(500); // 빠른 반응
}

void line_count(int count)
{
    int current_count = 0;
    bool crossed = false;

    while (true)
    {
        int leftEdge = digitalRead(SENSOR_LEFT);
        int rightEdge = digitalRead(SENSOR_RIGHT);

        // 교차점 감지
        if ((leftEdge == HIGH || rightEdge == HIGH) && !crossed)
        {
            crossed = true;
            current_count++;

            // 다음 교차점이 연속으로 감지되지 않도록 약간 밀어줌
            set_motor_speeds(115, 100);
            delay(300);

            if (count == 0 || current_count >= count)
            {
                car_brake(100); // 브레이크
                Serial.println("== 목표 교차점 도달: STOP ==");
                break;
            }

            // delay(150);  // 관성으로 다음 교차점까지 연속 감지 방지
        }

        // 교차점에서 빠져나오면 crossed 해제
        if (leftEdge == LOW && rightEdge == LOW)
        {
            crossed = false;
        }

        line_trace(); // 라인트레이싱 수행
    }
}

void turn_left()
{
    Serial.println("== 왼쪽 회전 시작 ==");

    // 1. 왼쪽 센서 감지될 때까지 좌회전
    while (digitalRead(SENSOR_LEFT) == LOW)
    {
        set_motor_speeds(-110, 100);
        delay(5);
    }

    Serial.println("== 왼쪽 센서 감지됨, 중앙 센서 대기 ==");

    // 2. 중앙 센서 감지될 때까지 계속 회전
    while (digitalRead(SENSOR_MID_R) == LOW)
    {
        set_motor_speeds(-110, 100);
        delay(5);
    }

    car_stop();
    Serial.println("== 중앙 센서 감지됨, 회전 종료 ==");

    delay(150); // 정지 후 약간 안정화

    // 3. 반대 방향으로 라인 재정렬
    while (digitalRead(SENSOR_MID_R) == HIGH)
    {
        set_motor_speeds(110, -100);
        delay(5);
    }

    car_stop();
    Serial.println("== 중앙 센서 미감지 → 정렬 완료, 직진 준비 완료 ==");
}

// 오른쪽 회전은 좌우 속도만 반대로 하면 돼
void turn_right()
{
    while (digitalRead(SENSOR_RIGHT) == LOW)
        set_motor_speeds(100, -100);

    while (digitalRead(SENSOR_MID_R) == LOW)
        set_motor_speeds(100, -100);

    car_stop();
    delay(150);

    while (digitalRead(SENSOR_MID_R) == HIGH)
        set_motor_speeds(-100, 100);

    car_stop();
}
