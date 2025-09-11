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
        set_motor_speeds(2*speed_fast, speed_slow);
    }
    else if (mid_l_detected && !mid_r_detected)
    {
        // State: [X, 1, 0, X] -> Slightly right of center, steer left
        set_motor_speeds(speed_slow, 2*speed_fast);
    }
    else
    {
        // State: [X, 1, 1, X] -> Line lost between middle sensors
        // Action: Stop for now. A search routine could be added here later.
        // car_stop();
    }
}

// '검정색은 싫어요' 함수: 교차로를 통과합니다.
// 양쪽 끝 센서가 모두 검은 선을 벗어날 때까지 중앙 센서로 라인을 추적하며 직진합니다.
void blno(int speed, bool apply_brake)
{
    // 양쪽 끝 센서 중 하나라도 라인을 감지하고 있는 동안 루프 실행
    while (digitalRead(SENSOR_LEFT) == LINE_DETECTED || digitalRead(SENSOR_RIGHT) == LINE_DETECTED)
    {
        forward_on(speed);
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
    bool left_edge_detected = (digitalRead(SENSOR_LEFT) == LINE_DETECTED);
    bool right_edge_detected = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED);
    if (left_edge_detected && right_edge_detected)
    {
        // Full intersection: cross it without braking and continue the loop.
        blno(OPT_SPEED, false);
    }

    while (true)
    {
        // 1. Follow the line for a small step using the middle sensors.
        line_track();

        // 2. Check the outer sensors for an intersection.
        bool left_edge_detected = (digitalRead(SENSOR_LEFT) == LINE_DETECTED);
        bool right_edge_detected = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED);

        if (left_edge_detected || right_edge_detected)
        {
            // T-junction or other line event detected.
            // Stop and then pivot to align both sensors with the line.
            
            // 1. Initial stop and stabilization
            car_brake(150);
            delay(100);

            // 2. Check which sensor is on the line and align the other.
            // Re-read sensors after brake to get the final resting state.
            bool final_left_detected = (digitalRead(SENSOR_LEFT) == LINE_DETECTED);
            bool final_right_detected = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED);
            
            // If both are on the line after the initial brake, no alignment is needed.
            align_on_intersection(true);

            // 4. Exit the main line_trace loop.
            break;
        }

        // A small delay to prevent the loop from running too fast and to allow for sensor reading stability.
        delay(1);
    }

    align_on_intersection();
    // car_brake(200);
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
            // 3. Intersection detected. Stop the robot and exit.
            car_brake(150);
            delay(100);
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

/**
 * @brief 관성으로 지나친 교차로 라인에 로봇을 다시 정렬합니다.
 * 로봇을 후진시켜 양쪽 끝 센서가 모두 라인 위에 위치하도록 조정합니다.
 */
void align_on_intersection(bool back_align) {
    // 이미 정렬된 경우 함수 종료
    if (digitalRead(SENSOR_LEFT) == LINE_DETECTED && digitalRead(SENSOR_RIGHT) == LINE_DETECTED) {
        return;
    }

    // 한쪽 센서라도 라인을 찾을 때까지 천천히 직진 후진
    if (back_align) {
        back_on(ALIGN_SPEED); // BUG FIX: Changed backward_on to back_on
    }
    else {
        forward_on(ALIGN_SPEED);
    }

    while (digitalRead(SENSOR_LEFT) != LINE_DETECTED && digitalRead(SENSOR_RIGHT) != LINE_DETECTED) {
        delay(1);
    }
    car_brake(100); // 첫 센서 감지 시 정지
    delay(50);

    // 한쪽은 감지했고, 나머지 한쪽을 마저 정렬
    bool left_on = (digitalRead(SENSOR_LEFT) == LINE_DETECTED || digitalRead(SENSOR_MID_L) == LINE_DETECTED);
    bool right_on = (digitalRead(SENSOR_RIGHT) == LINE_DETECTED || digitalRead(SENSOR_MID_R) == LINE_DETECTED);

    if (left_on && !right_on) {
        // 왼쪽은 감지, 오른쪽은 못했을 경우: 오른쪽만 뒤로 돌려 마저 정렬
        set_motor_speeds(0, -ALIGN_SPEED);
        while (! (digitalRead(SENSOR_RIGHT) == LINE_DETECTED && digitalRead(SENSOR_MID_R) == LINE_DETECTED)) {
            delay(1);
        }
    } else if (!left_on && right_on) {
        // 오른쪽은 감지, 왼쪽은 못했을 경우: 왼쪽만 뒤로 돌려 마저 정렬
        set_motor_speeds(-ALIGN_SPEED, 0);
        while (! (digitalRead(SENSOR_LEFT) == LINE_DETECTED && digitalRead(SENSOR_MID_L) == LINE_DETECTED)) {
            delay(1);
        }
    }

    // 양쪽 모두 정렬 완료. 최종 정지.
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
