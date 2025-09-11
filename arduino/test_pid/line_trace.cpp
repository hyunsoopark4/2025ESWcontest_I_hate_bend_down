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
