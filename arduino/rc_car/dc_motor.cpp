// DC모터
#define R_IA 6 // Right Motor IA (must be PWM output)
#define R_IB 11
#define L_IA 3 // Left Motor IA (must be PWM output)
#define L_IB 5
#include <Arduino.h>
#include "dc_motor.h"

void spin_left_on(int speed)
{
    analogWrite(R_IA, speed); // right motor in forward direction
    analogWrite(R_IB, 0);
    analogWrite(L_IA, speed); // left motor in backward direction
    analogWrite(L_IB, 0);
}

void spin_right_on(int speed)
{
    analogWrite(R_IA, 0);
    analogWrite(R_IB, speed);
    analogWrite(L_IA, 0);
    analogWrite(L_IB, speed);
}

void car_stop()
{
    analogWrite(R_IA, 0);
    analogWrite(R_IB, 0);
    analogWrite(L_IA, 0);
    analogWrite(L_IB, 0);
}

void car_brake(int time)
{
    back_on(OPT_SPEED / 2);
    delay(time);
    car_stop();
}

// void turn_left(int duration)
// {
//     spin_left_on();
//     delay(duration);
//     car_stop();
//     return;
// }

// void turn_right(int duration)
// {
//     spin_right_on();
//     delay(duration);
//     car_stop();
//     return;
// }

// void turn_back(int duration)
// {
//     spin_right_on();
//     delay(duration);
//     car_stop();
//     return;
// }

void r_motor_on(int speed)
{
    analogWrite(R_IA, speed); // right motor (+ direction)
    analogWrite(R_IB, 0);
    analogWrite(L_IA, 0);
    analogWrite(L_IB, 0);
}

void l_motor_on(int speed)
{
    analogWrite(R_IA, 0);
    analogWrite(R_IB, 0);
    analogWrite(L_IA, speed); // left motor (- direction)
    analogWrite(L_IB, 0);
}
void forward_on(int speed)
{
    analogWrite(R_IA, speed);
    analogWrite(R_IB, 0);
    analogWrite(L_IA, 0);
    analogWrite(L_IB, speed);
}

void back_on(int speed)
{
    analogWrite(R_IA, 0);
    analogWrite(R_IB, speed);
    analogWrite(L_IA, speed);
    analogWrite(L_IB, 0);
}

void set_motor_speeds(int l_speed, int r_speed) {
    analogWrite(R_IA, r_speed);
    analogWrite(R_IB, 0);
    analogWrite(L_IA, 0);
    analogWrite(L_IB, l_speed);
}