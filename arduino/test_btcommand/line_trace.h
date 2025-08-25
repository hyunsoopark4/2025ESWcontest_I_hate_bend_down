#ifndef LINE_TRACE_H
#define LINE_TRACE_H

void line_trace();  // 일상 주행
void line_trace_torque(); // 목표물 습득 후 토크감 있는 주행
void line_count(int count);
void turn_left();
void turn_right();



// 외부에서 사용할 수도 있도록 공개
extern int node_count;
extern bool crossed;

#endif
