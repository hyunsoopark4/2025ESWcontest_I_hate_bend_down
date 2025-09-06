#ifndef LINE_TRACE_H
#define LINE_TRACE_H

// IR Sensor Pins (LOW when line is detected)
#define SENSOR_LEFT 7
#define SENSOR_MID_L 8
#define SENSOR_MID_R 9
#define SENSOR_RIGHT 10

// Readability improvement
#define LINE_DETECTED HIGH

#define SPEED_FWD 130
#define SPEED_BWD -100

//170 , 30 low battery
#define SPEED_FAST 170
#define SPEED_SLOW 50

#define SPEED_TORQUE_FAST 255
#define SPEED_TORQUE_SLOW 70

// 토크 회전을 위한 새로운 속도 정의 추가
#define SPEED_TURN_TORQUE_FWD 220  // 최대 속도 (255에서 하향 조정)
#define SPEED_TURN_TORQUE_BWD -150 // 더 강한 역방향 속도

// 함수 선언
void line_trace();  // 일상 주행
void line_trace_torque(); // 목표물 습득 후 토크감 있는 주행
void turn_left();
void turn_right();
// line_track의 매개변수에 기본값 지정
void line_track(int speed_fast = SPEED_FAST, int speed_slow = SPEED_SLOW);

// 토크 회전 함수 추가
void torque_turn_left();
void torque_turn_right();

// 외부에서 사용할 수도 있도록 공개
extern int node_count;  
extern bool crossed;


void turn_left_stable();
void turn_right_stable();

#endif
