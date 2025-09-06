#include <Arduino.h>
#include "line_trace.h"
#include "dc_motor.h" // set_motor_speeds 직접 사용

bool lastState = false;

// Proportional control line tracking using the two middle sensors
void line_track(int speed_fast, int speed_slow)
{
    bool mid_l_detected = (digitalRead(SENSOR_MID_L) == LINE_DETECTED);
    bool mid_r_detected = (digitalRead(SENSOR_MID_R) == LINE_DETECTED);

    if (mid_l_detected && mid_r_detected) {
        // State: [X, 1, 1, X] -> Perfect center
        set_motor_speeds(speed_fast, speed_fast);
    } else if (mid_l_detected && !mid_r_detected) {
        // State: [X, 1, 0, X] -> Slightly left of center, steer right
        set_motor_speeds(speed_fast, speed_slow);
    } else if (!mid_l_detected && mid_r_detected) {
        // State: [X, 0, 1, X] -> Slightly right of center, steer left
        set_motor_speeds(speed_slow, speed_fast);
    } else {
        // State: [X, 0, 0, X] -> Line lost between middle sensors
        // Action: Stop for now. A search routine could be added here later.
        car_stop();
    }
}

// Moves the robot along the line until an intersection is detected by outer sensors.
void line_trace()
{
    while (true)
    {
        // 1. Follow the line for a small step using the middle sensors.
        line_track();

        // 2. Check the outer sensors for an intersection.
        bool left_edge_detected = (digitalRead(SENSOR_LEFT) == LINE_DETECTED);
        bool right_edge_detected = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED);

        if (left_edge_detected || right_edge_detected)
        {
            // 3. Intersection detected. Stop the robot and exit the function.
            car_brake(150); // Apply brake for a firm stop
            delay(100);     // Wait for the robot to settle
            return;

        }

        // A small delay to prevent the loop from running too fast and to allow for sensor reading stability.
        delay(1);
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

void turn_left()
{
    Serial.println(F("== New Turn Left Sequence Initiated =="));

    // Stage 1: Pre-Alignment on the current line for stability.
    // Instead of a complex wobble-check, we use line_track() for a short duration.
    Serial.println(F("1. Aligning on current line..."));
    for (int i = 0; i < 100; i++) {
        line_track();
        delay(1);
    }
    car_brake(100);
    delay(200);

    // Stage 2: Move forward to position the robot's pivot point in the center of the intersection.
    Serial.println(F("2. Moving to intersection center..."));
    forward_on();
    delay(MOVE_TO_CENTER_DURATION); // This constant is in dc_motor.h and needs tuning.

    // Stage 3: Pivot turn using multiple sensors to find the new line.
    Serial.println(F("3a. Pivoting left to find new line (outer sensor)..."));
    spin_left_on();
    while (digitalRead(SENSOR_LEFT) != LINE_DETECTED) {
        delay(1);
    }

    Serial.println(F("3b. ...fine-tuning (mid-left sensor)..."));
    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED) {
        delay(1);
    }

    Serial.println(F("3c. ...slowing down (mid-right sensor)..."));
    spin_left_on(TURN_SPEED_SLOW);
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED) {
        delay(1);
    }

    // Stage 4: Stop and stabilize.
    Serial.println(F("4. Turn complete. Final stop."));
    car_brake(150);
    delay(100);
}

void turn_right()
{
    Serial.println(F("== New Turn Right Sequence Initiated =="));

    // Stage 1: Pre-Alignment on the current line for stability.
    Serial.println(F("1. Aligning on current line..."));
    for (int i = 0; i < 100; i++) {
        line_track();
        delay(1);
    }
    car_brake(100);
    delay(200);

    // Stage 2: Move forward to position the robot's pivot point in the center of the intersection.
    Serial.println(F("2. Moving to intersection center..."));
    forward_on();
    delay(MOVE_TO_CENTER_DURATION); // This constant is in dc_motor.h and needs tuning.

    // Stage 3: Pivot turn using multiple sensors to find the new line.
    Serial.println(F("3a. Pivoting right to find new line (outer sensor)..."));
    spin_right_on();
    while (digitalRead(SENSOR_RIGHT) != LINE_DETECTED) {
        delay(1);
    }

    Serial.println(F("3b. ...fine-tuning (mid-right sensor)..."));
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED) {
        delay(1);
    }

    Serial.println(F("3c. ...slowing down (mid-left sensor)..."));
    spin_right_on(TURN_SPEED_SLOW);
    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED) {
        delay(1);
    }

    // Stage 4: Stop and stabilize.
    Serial.println(F("4. Turn complete. Final stop."));
    car_brake(150);
    delay(100);
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