#ifndef LINE_TRACE_H
#define LINE_TRACE_H

#define SPEED_FWD 150
#define SPEED_BWD 0

//170 , 30 low battery
#define SPEED_FAST 150
#define SPEED_SLOW 20

void line_trace();  // 일상 주행
void line_trace_torque(); // 목표물 습득 후 토크감 있는 주행
void turn_left(int speed_turn_fwd = SPEED_FWD, int speed_turn_bwd = SPEED_BWD);
void turn_right(int speed_turn_fwd = SPEED_FWD, int speed_turn_bwd = SPEED_BWD);
void line_track(int speed_fast = SPEED_FAST, int speed_slow = SPEED_SLOW);


// 외부에서 사용할 수도 있도록 공개
extern int node_count;
extern bool crossed;

#endif
