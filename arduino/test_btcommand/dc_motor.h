#ifndef DC_MOTOR_H
#define DC_MOTOR_H
#include <Arduino.h>

#define OPT_SPEED 200         // DC Motor speed 200 recommended (Min.150 ~ Max.255)
#define TURNBACK_DURATION 425 //
#define TURN_DURATION 150     // turn in TURN_DURATION time

// void turn_left(int duration = TURN_DURATION);
// void turn_right(int duration = TURN_DURATION);
// void turn_back(int duration = TURNBACK_DURATION);
void car_stop();
void car_brake(int time = 100);
void r_motor_on(int speed = OPT_SPEED);
void l_motor_on(int speed = OPT_SPEED);
void forward_on(int speed = OPT_SPEED);
void back_on(int speed = OPT_SPEED);
void spin_left_on(int l_speed = OPT_SPEED, int r_speed = OPT_SPEED);
void spin_right_on(int l_speed = OPT_SPEED, int r_speed = OPT_SPEED);
void set_motor_speeds(int r_speed, int l_speed);

#endif
