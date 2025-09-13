#include <Arduino.h>
#include "line_trace.h"
#include "dc_motor.h" // set_motor_speeds 직접 사용
#include "pid.h"      // PID 라인트레이싱 사용

// PID가 센서 초기화와 기본 라인 추적을 담당하므로 line_trace_init과 line_track은 제거됨

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

// PID 기반 라인 추적 (기존 line_trace를 대체, line_trace_torque 통합)
void line_trace(int base_speed)
{
    // 1단계: 교차로 통과 (blno)
    Serial.println("Step 1: Cross intersection");
    blno(base_speed > 150 ? base_speed : OPT_SPEED, false);

    // 2단계: PID 라인트레이싱
    Serial.println("Step 2: PID line tracing");
    line_pid.pid_linetrace(base_speed);

    // 3단계: 역방향 PID 라인트레이싱 (위치 보정)
    Serial.println("Step 3: Reverse PID correction");
    int reverse_speed = base_speed > 150 ? 100 : 80; // 토크 모드일 때 더 높은 역방향 속도
    line_pid.reverse_pid_linetrace(reverse_speed, 1000);

    Serial.println("Line trace sequence complete");
}

void turn_left()
{
    int speed = TURN_SPEED;

    // 1. 현재 라인에서 벗어날 때까지 회전
    spin_left_on(speed);
    while (digitalRead(SENSOR_LEFT) == LINE_DETECTED)
    {
        delay(1);
    }

    // 2. 안전 마진을 위해 0.3초 추가 회전
    delay(300);

    // 3. 다음 라인을 중앙 센서가 감지할 때까지 회전
    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED)
    {
        delay(1);
    }

    // 4. 회전 정지 및 최종 정렬
    car_brake(300);
    line_pid.align_intersection(ALIGN_SPEED); // 정밀 정렬
    
}

void turn_right()
{
    int speed = TURN_SPEED;

    // 1. 현재 라인에서 벗어날 때까지 회전
    spin_right_on(speed);
    while (digitalRead(SENSOR_RIGHT) == LINE_DETECTED)
    {
        delay(1);
    }

    // 2. 안전 마진을 위해 0.3초 추가 회전
    delay(300);

    // 3. 다음 라인을 중앙 센서가 감지할 때까지 회전
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED)
    {
        delay(1);
    }

    // 4. 회전 정지 및 최종 정렬
    car_brake(300);
    line_pid.align_intersection(ALIGN_SPEED); // 정밀 정렬


}

// --- Torque Mode Functions ---
// The logic is identical to the standard functions but uses higher torque speed constants.

// 토크 모드용 간단한 비례 제어 (PID 대신 사용)
void torque_line_track(int speed_fast, int speed_slow)
{
    bool mid_l_detected = (digitalRead(SENSOR_MID_L) == LINE_DETECTED);
    bool mid_r_detected = (digitalRead(SENSOR_MID_R) == LINE_DETECTED);

    if (!mid_l_detected && !mid_r_detected)
    {
        // Perfect center
        set_motor_speeds(speed_fast, speed_fast);
    }
    else if (!mid_l_detected && mid_r_detected)
    {
        // Slightly left of center, steer right
        set_motor_speeds(2 * speed_fast, speed_slow);
    }
    else if (mid_l_detected && !mid_r_detected)
    {
        // Slightly right of center, steer left
        set_motor_speeds(speed_slow, 2 * speed_fast);
    }
    // 토크 모드에서는 센서 오류 시 정지하지 않고 계속 진행
}

void torque_turn_left()
{
    // Stage 1: Pre-Alignment
    for (int i = 0; i < 75; i++)
    { // Align for a bit longer due to higher momentum
        torque_line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW);
        delay(1);
    }
    car_brake(100);
    delay(100);

    // Stage 2: Move to Center
    forward_on(SPEED_TURN_TORQUE_FWD);
    delay(MOVE_TO_CENTER_DURATION);

    // Stage 3: Pivot Turn
    spin_left_on(SPEED_TURN_TORQUE_FWD);
    while (digitalRead(SENSOR_LEFT) != LINE_DETECTED)
    {
        delay(1);
    }

    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED)
    {
        delay(1);
    }

    spin_left_on(SPEED_TURN_TORQUE_FWD / 2); // Use half torque speed for slow part
    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED)
    {
        delay(1);
    }

    // Stage 4: Stop
    car_brake(150);
    delay(100);
}

void torque_turn_right()
{
    // Stage 1: Pre-Alignment
    for (int i = 0; i < 75; i++)
    { // Align for a bit longer due to higher momentum
        torque_line_track(SPEED_TORQUE_FAST, SPEED_TORQUE_SLOW);
        delay(1);
    }
    car_brake(100);
    delay(100);

    // Stage 2: Move to Center
    forward_on(SPEED_TURN_TORQUE_FWD);
    delay(MOVE_TO_CENTER_DURATION);

    // Stage 3: Pivot Turn
    spin_right_on(SPEED_TURN_TORQUE_FWD);
    while (digitalRead(SENSOR_RIGHT) != LINE_DETECTED)
    {
        delay(1);
    }

    while (digitalRead(SENSOR_MID_R) != LINE_DETECTED)
    {
        delay(1);
    }

    spin_right_on(SPEED_TURN_TORQUE_FWD / 2); // Use half torque speed for slow part
    while (digitalRead(SENSOR_MID_L) != LINE_DETECTED)
    {
        delay(1);
    }

    // Stage 4: Stop
    car_brake(150);
    delay(100);
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
        while (!(digitalRead(SENSOR_RIGHT) == LINE_DETECTED && digitalRead(SENSOR_MID_R) == LINE_DETECTED)) {
            delay(1);
        }
    }
    else if (!left_on && right_on) {
        // 오른쪽은 감지, 왼쪽은 못했을 경우: 왼쪽만 뒤로 돌려 마저 정렬
        set_motor_speeds(-ALIGN_SPEED, 0);
        while (!(digitalRead(SENSOR_LEFT) == LINE_DETECTED && digitalRead(SENSOR_MID_L) == LINE_DETECTED)) {
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
