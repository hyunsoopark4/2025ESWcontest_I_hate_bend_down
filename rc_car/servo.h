#ifndef SERVO_H
#define SERVO_H
#include <Arduino.h>

void servo_setup();
void servo_front();
void servo_left();
void servo_right();
void servo_write(int degree);

#endif