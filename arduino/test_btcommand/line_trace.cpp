#include <Arduino.h>
#include "line_trace.h"
#include "dc_motor.h" // set_motor_speeds 직접 사용
#include "mpu.h"      // mpu_get_yaw_difference 함수 사용
#include "navigation.h" // NORTH, EAST, SOUTH, WEST 정의 사용

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
            Serial.println("== Detected: Turn right ==");
        }
    }
    else
    { // 라인 벗어남
        set_motor_speeds(speed_slow, speed_fast); // 좌측 회전
        if (lastState)
        {
            lastState = false;
            Serial.println("== Not detected: Turn left ==");
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
          line_stabilize();
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
    Serial.println("== Left turn started ==");

    // 왼쪽 바퀴는 역방향(속도 낮게), 오른쪽 바퀴는 정방향으로 회전
    int reverse_speed = -70;  // 역방향 속도를 직접 지정
    
    // 1. 왼쪽 센서가 라인을 감지할 때까지 좌회전
    while (digitalRead(SENSOR_LEFT) == LOW)
    {
        set_motor_speeds(reverse_speed, speed_turn_fwd);
        delay(5);
    }

    Serial.println("== Left sensor detected, waiting for center sensor ==");

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
    Serial.println("== Turn and alignment completed ==");
}

void turn_right(int speed_turn_fwd, int speed_turn_bwd)
{
    Serial.println("== Right turn started ==");

    // 오른쪽 바퀴는 역방향(속도 낮게), 왼쪽 바퀴는 정방향으로 회전
    int reverse_speed = -70;  // 역방향 속도를 직접 지정
    
    // 1. 오른쪽 센서 감지될 때까지 우회전
    while (digitalRead(SENSOR_RIGHT) == LOW)
    {
        set_motor_speeds(speed_turn_fwd, reverse_speed);
        delay(5);
    }

    Serial.println("== Right sensor detected, waiting for center sensor ==");

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
    Serial.println("== Turn and alignment completed ==");
}

// MPU를 활용한 방향 안정화 함수
void line_stabilize(int aim) {
    const float TOLERANCE = 2.5;  // 허용 오차 (도)
    const int SLOW_SPEED = 115;    // 천천히 회전하는 속도

    float current_yaw = mpu_get_yaw_difference();
    float target_angle = 0.0;
    
    if (aim == 0) {
        // 가장 가까운 90도 배수로 정렬 (0, 90, -90, 180/-180)
        if (current_yaw >= -45 && current_yaw <= 45) {
            target_angle = 0.0;
        } else if (current_yaw > 45 && current_yaw <= 135) {
            target_angle = 90.0;
        } else if (current_yaw > 135 || current_yaw <= -135) {
            target_angle = 180.0;
        } else { // current_yaw < -45 && current_yaw > -135
            target_angle = -90.0;
        }
    } else {
        // navigation.h의 방향을 각도로 변환 (NORTH=0도 기준)
        switch (aim) {
            case TARGET_NORTH:          
                target_angle = 0.0; break;
            case TARGET_NORTH_EAST: 
                target_angle = 45.0; break;
            case TARGET_EAST:           
                target_angle = 90.0; break;
            case TARGET_SOUTH_EAST: 
                target_angle = 135.0; break;
            case TARGET_SOUTH:          
                target_angle = 180.0; break;
            case TARGET_SOUTH_WEST: 
                target_angle = -135.0; break;
            case TARGET_WEST:           
                target_angle = -90.0; break;
            case TARGET_NORTH_WEST: 
                target_angle = -45.0; break;
            default:             
                target_angle = 0.0; break;
        }
    }
    
    Serial.print("== Stabilizing to angle: ");
    Serial.println(target_angle);
    
    while (true) {
        current_yaw = mpu_get_yaw_difference();
        float angle_diff = target_angle - current_yaw;

        // 각도 차이를 -180 ~ 180 범위로 정규화
        while (angle_diff > 180.0) angle_diff -= 360.0;
        while (angle_diff < -180.0) angle_diff += 360.0;

        Serial.print("angle difference: ");
        Serial.println(angle_diff);

        // 목표 각도에 도달했는지 확인
        if (abs(angle_diff) <= TOLERANCE) {
            Serial.println("== Angle stabilization completed ==");
            break;
        }
        
        // 회전 방향 결정 및 실행
        if (angle_diff > 0) {
            // 시계 방향으로 회전 (오른쪽)
            spin_right_on(SLOW_SPEED);
            delay(50);
            spin_left_on(OPT_SPEED);
        } else {
            // 반시계 방향으로 회전 (왼쪽)
            spin_left_on(SLOW_SPEED);
            delay(50);
            spin_right_on(OPT_SPEED);
        }
        car_stop();
    }
}