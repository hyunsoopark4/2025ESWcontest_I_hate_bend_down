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

extern int node_count;
extern bool crossed;

#endif
