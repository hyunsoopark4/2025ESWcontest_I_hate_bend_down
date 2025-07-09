#ifndef ULTRASONIC_H
#define ULTRASONIC_H
#include <Arduino.h>

bool is_obstacle();
int get_ultrasonic_cm();
int ultrasonic_setup();

#endif