#ifndef PID_LINE_H
#define PID_LINE_H

#include <Arduino.h>

// 센서 핀 정의
#define SENSOR_LEFT    7
#define SENSOR_MID_L   9
#define SENSOR_MID_R   10
#define SENSOR_RIGHT   8

void pid_line_setup();
void pid_linetrace(int target_node);

// PID 상태 변수
extern int node_count;
extern bool crossed;

// 센서 감지 시간 측정을 위한 변수
extern unsigned long sensor_timers[4];
extern bool sensor_triggered[4];
extern int sensor_durations[4];


#endif