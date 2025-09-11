#include <Arduino.h>
#include "line_trace.h"
#include "dc_motor.h" // set_motor_speeds 직접 사용

void line_trace_init()
{
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
}

bool lastState = false;

// Proportional control line tracking using the two middle sensors
void line_track(int speed_fast, int speed_slow)
{
    bool mid_l_detected = (digitalRead(SENSOR_MID_L) == LINE_DETECTED);
    bool mid_r_detected = (digitalRead(SENSOR_MID_R) == LINE_DETECTED);

    if (!mid_l_detected && !mid_r_detected)
    {
        // State: [X, 0, 0, X] -> Perfect center
        set_motor_speeds(speed_fast, speed_fast);
    }
    else if (!mid_l_detected && mid_r_detected)
    {
        // State: [X, 0, 1, X] -> Slightly left of center, steer right
        set_motor_speeds(speed_fast, speed_slow);
    }
    else if (mid_l_detected && !mid_r_detected)
    {
        // State: [X, 1, 0, X] -> Slightly right of center, steer left
        set_motor_speeds(speed_slow, speed_fast);
    }
    else
    {
        // State: [X, 1, 1, X] -> Line lost between middle sensors
        // Action: Stop for now. A search routine could be added here later.
        // car_stop();
    }
}

// '검정색은 싫어요' 함수: 교차로를 통과합니다.
// 양쪽 끝 센서가 모두 검은 선을 벗어날 때까지 라인을 추적하며 직진합니다.
void blno(int speed, bool apply_brake)
{
    // 양쪽 끝 센서 중 하나라도 라인을 감지하고 있는 동안 루프 실행
    while (digitalRead(SENSOR_LEFT) == LINE_DETECTED || digitalRead(SENSOR_RIGHT) == LINE_DETECTED)
    {
        line_track(); // 라인을 따라가며 교차로 통과
        delay(1);
    }

    if (apply_brake)
    {
        // 교차로를 완전히 통과하기 위해 약간 전진
        forward_on(speed);
        delay(50); // 필요에 따라 시간 조절
        car_brake(100);
    }
}

// Moves the robot along the line until an intersection is detected by outer sensors.
void line_trace()
{
    bool left_edge_detected_initial = (digitalRead(SENSOR_LEFT) == LINE_DETECTED);
    bool right_edge_detected_initial = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED);
    if (left_edge_detected_initial && right_edge_detected_initial)
    {
        // Full intersection: cross it without braking and continue the loop.
        blno(OPT_SPEED, false);
    }

    while (true)
    {
        // 1. Always follow the line with the rear sensors.
        line_track();

        // 2. Check the front sensors for an intersection.
        bool left_edge_detected = (digitalRead(SENSOR_LEFT) == LINE_DETECTED);
        bool right_edge_detected = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED);

        if (left_edge_detected || right_edge_detected)
        {
            // 3. Intersection detected by front sensors.
            // Continue line tracking for a fixed duration to move the pivot point to the center.
            unsigned long startTime = millis();
            while (millis() - startTime < MOVE_TO_CENTER_DURATION)
            {
                line_track(); // Keep tracking the line while moving forward
                delay(1);
            }

            // 4. Stop and stabilize.
            car_brake(150);
            delay(100);

            // 5. Align perfectly on the intersection line.
            align_on_intersection(false);

            // 6. Exit the main line_trace loop. The robot is now aligned at the intersection.
            break;
        }

        // A small delay to prevent the loop from running too fast and to allow for sensor reading stability.
        delay(1);
    }
    return;
}

void turn_left()
{
    Serial.println(F("== New Turn Left Sequence Initiated =="));



    spin_left_on();
    while (digitalRead(SENSOR_LEFT) == LINE_DETECTED)
    {
        delay(1);
    }
    delay(50); // 추가 지연으로 관성 보정
    while (digitalRead(SENSOR_LEFT) != LINE_DETECTED)
    {
        delay(1);
    }

    car_brake(100);
    delay(300);

    /*
        // Stage 1: Pre-Alignment on the current line for stability.
        Serial.println(F("1. Aligning on current line..."));
        for (int i = 0; i < 100; i++) {
            line_track();
            delay(1);
        }
        car_brake(100);
        delay(100);

        // Stage 2: Move forward to position the robot's pivot point in the center of the intersection.
        // Serial.println(F("2. Moving to intersection center..."));
        // forward_on();
        // delay(MOVE_TO_CENTER_DURATION); // This constant is in dc_motor.h and needs tuning.

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
        */
}

void turn_right()
{

    spin_right_on();
    while (digitalRead(SENSOR_RIGHT) == LINE_DETECTED)
    {
        delay(1);
    }

    delay(50);
    
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED)
    {
        delay(1);
    }

    car_brake(300);
    
}

// --- Torque Mode Functions ---
// The logic is identical to the standard functions but uses higher torque speed constants.

void line_trace_torque()
{
    while (true)
    {
        // 1. Follow the line with higher torque speeds.
        line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW);

        // 2. Check the outer sensors for an intersection.
        bool left_edge_detected = (digitalRead(SENSOR_LEFT) == LINE_DETECTED);
        bool right_edge_detected = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED);

        if (left_edge_detected || right_edge_detected)
        {
            // 3. Intersection detected by front sensors.
            // Continue line tracking for a fixed duration to move the pivot point to the center.
            unsigned long startTime = millis();
            while (millis() - startTime < MOVE_TO_CENTER_DURATION)
            {
                line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW); // Keep tracking with torque speeds
                delay(1);
            }

            // 4. Stop and stabilize.
            car_brake(150);
            delay(100);

            // 5. Exit the function. The robot is now at the intersection.
            return;
        }
        delay(1);
    }
}

void torque_turn_left()
{
    Serial.println(F("== Torque Turn Left Sequence Initiated =="));

    // Stage 1: Pre-Alignment
    Serial.println(F("1. Aligning on current line (torque)..."));
    for (int i = 0; i < 75; i++)
    { // Align for a bit longer due to higher momentum
        line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW);
        delay(1);
    }
    car_brake(100);
    delay(100);

    // Stage 2: Move to Center
    Serial.println(F("2. Moving to intersection center (torque)..."));
    forward_on(SPEED_TURN_TORQUE_FWD);
    delay(MOVE_TO_CENTER_DURATION);

    // Stage 3: Pivot Turn
    Serial.println(F("3a. Pivoting left (torque speed)..."));
    spin_left_on(SPEED_TURN_TORQUE_FWD);
    while (digitalRead(SENSOR_LEFT) != LINE_DETECTED)
    {
        delay(1);
    }

    Serial.println(F("3b. ...fine-tuning (mid-left sensor)..."));
    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED)
    {
        delay(1);
    }

    Serial.println(F("3c. ...slowing down (mid-right sensor)..."));
    spin_left_on(SPEED_TURN_TORQUE_FWD / 2); // Use half torque speed for slow part
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED)
    {
        delay(1);
    }

    // Stage 4: Stop
    Serial.println(F("4. Turn complete. Final stop."));
    car_brake(150);
    delay(100);
}

void torque_turn_right()
{
    Serial.println(F("== Torque Turn Right Sequence Initiated =="));

    // Stage 1: Pre-Alignment
    Serial.println(F("1. Aligning on current line (torque)..."));
    for (int i = 0; i < 75; i++)
    { // Align for a bit longer due to higher momentum
        line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW);
        delay(1);
    }
    car_brake(100);
    delay(100);

    // Stage 2: Move to Center
    Serial.println(F("2. Moving to intersection center (torque)..."));
    forward_on(SPEED_TURN_TORQUE_FWD);
    delay(MOVE_TO_CENTER_DURATION);

    // Stage 3: Pivot Turn
    Serial.println(F("3a. Pivoting right (torque speed)..."));
    spin_right_on(SPEED_TURN_TORQUE_FWD);
    while (digitalRead(SENSOR_RIGHT) != LINE_DETECTED)
    {
        delay(1);
    }

    Serial.println(F("3b. ...fine-tuning (mid-right sensor)..."));
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED)
    {
        delay(1);
    }

    Serial.println(F("3c. ...slowing down (mid-left sensor)..."));
    spin_right_on(SPEED_TURN_TORQUE_FWD / 2); // Use half torque speed for slow part
    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED)
    {
        delay(1);
    }

    // Stage 4: Stop
    Serial.println(F("4. Turn complete. Final stop."));
    car_brake(150);
    delay(100);
}

/**
 * @brief 교차로 라인에 로봇을 정렬합니다.
 * 라인을 벗어난 쪽의 바퀴만 전진시켜 자세를 바로잡습니다.
 */
void align_on_intersection(bool back_align) {
    // 1. Check if already aligned. If so, do nothing and exit.
    if (digitalRead(SENSOR_LEFT) == LINE_DETECTED && digitalRead(SENSOR_RIGHT) == LINE_DETECTED) {
        return;
    }

    // 2. Determine which side is on the line.
    // Note: This check uses outer and middle sensors for robustness.
    bool left_on = (digitalRead(SENSOR_LEFT) == LINE_DETECTED || digitalRead(SENSOR_MID_L) == LINE_DETECTED);
    bool right_on = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED || digitalRead(SENSOR_MID_R) == LINE_DETECTED);

    // 3. Pivot the unaligned side FORWARD until it finds the line.
    if (left_on && !right_on) {
        // Left side is on, right side is not. Pivot right wheel forward.
        set_motor_speeds(0, ALIGN_SPEED);
        // Wait until the right-side sensors detect the line.
        while (digitalRead(SENSOR_RIGHT) != LINE_DETECTED && digitalRead(SENSOR_MID_R) != LINE_DETECTED) {
            delay(1);
        }
    } else if (!left_on && right_on) {
        // Right side is on, left side is not. Pivot left wheel forward.
        set_motor_speeds(ALIGN_SPEED, 0);
        // Wait until the left-side sensors detect the line.
        while (digitalRead(SENSOR_LEFT) != LINE_DETECTED && digitalRead(SENSOR_MID_L) != LINE_DETECTED) {
            delay(1);
        }
    }

    // 4. Both sides are now aligned. Apply a final brake to stop firmly.
    car_brake(200);
}

void find_line() {
    // 로봇이 라인을 찾을 때까지 천천히 전진
    forward_on(ALIGN_SPEED);
    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED && digitalRead(SENSOR_MID_R) != LINE_DETECTED) {
        delay(1);
    }
    car_brake(100);
    delay(50);

    // 중앙 센서가 라인을 찾았으므로, 양쪽 끝 센서가 모두 라인 위에 위치하도록 정렬
    align_on_intersection();
}

/*
void align_to_line() {
    // 양쪽 끝 센서가 모두 라인을 찾을 때까지 천천히 전진
    find_line();
    forward_on(ALIGN_SPEED);
    while (digitalRead(SENSOR_LEFT) != LINE_DETECTED || digitalRead(SENSOR_RIGHT) != LINE_DETECTED) {
        delay(1);
    }
    car_brake(100);
    delay(50);
}
*/

// 1개 센서로 라인의 한쪽 날을 따라가는 함수
void line_track_one_sensor(int speed_fast, int speed_slow)
{
    // SENSOR_MID_R 센서 하나만 사용합니다.
    int center = digitalRead(SENSOR_MID_R);
    static bool lastState = false; // 로그 출력을 위해 상태 저장

    // LINE_DETECTED가 HIGH이므로, HIGH일 때 라인을 감지한 것입니다.
    if (center == LINE_DETECTED) // 라인 위 (HIGH)
    {
        // 제공된 로직에 따라 우측으로 회전
        set_motor_speeds(speed_fast, speed_slow);
        if (!lastState)
        {
            lastState = true;
            Serial.println("== 1-Sens: 감지(HIGH) -> 우회전 ==");
        }
    }
    else // 라인 벗어남 (LOW)
    {
        // 제공된 로직에 따라 좌측으로 회전
        set_motor_speeds(speed_slow, speed_fast);
        if (lastState)
        {
            lastState = false;
            Serial.println("== 1-Sens: 미감지(LOW) -> 좌회전 ==");
        }
    }
    delayMicroseconds(500);
}

// 1개 센서 라인트레이싱을 실행하는 메인 함수
void trace_one_sens()
{
    int leftEdge = digitalRead(SENSOR_LEFT);
    int rightEdge = digitalRead(SENSOR_RIGHT);

    // 시작 시 교차로에 있다면 통과
    while (leftEdge == LINE_DETECTED || rightEdge == LINE_DETECTED)
    {
        line_track_one_sensor(OPT_SPEED, LINE_TRACE_SLOW_SPEED);
        delay(1);
        leftEdge = digitalRead(SENSOR_LEFT);
        rightEdge = digitalRead(SENSOR_RIGHT);
    }

    // 라인을 찾을 때까지 트레이싱 시작
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED)
    {
        line_track_one_sensor(OPT_SPEED, LINE_TRACE_SLOW_SPEED);
        delay(1);
    }

    // 교차점을 만날 때까지 계속 라인트레이싱
    while (true)
    {
        leftEdge = digitalRead(SENSOR_LEFT);
        rightEdge = digitalRead(SENSOR_RIGHT);

        // 교차점 감지 로직
        if (leftEdge == LINE_DETECTED || rightEdge == LINE_DETECTED)
        {
            // 교차점 통과를 위해 약간 더 저속 트레이싱
            for (int i = 0; i < 20; i++)
            {
                line_track_one_sensor(LINE_TRACE_SLOW_SPEED, LINE_TRACE_SLOW_SPEED / 2);
                delay(5);
            }
            if (leftEdge != digitalRead(SENSOR_LEFT) && rightEdge != digitalRead(SENSOR_RIGHT))
                continue;

            // 급정지로 위치 안정화
            car_brake(200);
            delay(300);
            return; // 함수 종료
        }

        // 일반 라인트레이싱
        line_track_one_sensor(OPT_SPEED, LINE_TRACE_SLOW_SPEED);
    }
}