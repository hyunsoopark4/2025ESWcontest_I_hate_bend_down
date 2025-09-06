#include "dc_motor.h"

// Private helper function to control a single motor
// speed > 0: forward, speed < 0: backward, speed == 0: stop
void control_motor(int pin_ia, int pin_ib, int speed) {
    if (speed > 0) {
        analogWrite(pin_ia, speed);
        analogWrite(pin_ib, 0);
    } else if (speed < 0) {
        analogWrite(pin_ia, 0);
        analogWrite(pin_ib, -speed); // Use absolute value for speed
    } else {
        analogWrite(pin_ia, 0);
        analogWrite(pin_ib, 0);
    }
}

// Sets the speed for the left and right sides of the 4WD robot.
void set_motor_speeds(int l_speed, int r_speed)
{
    // Control left side motors
    control_motor(L_IA_F, L_IB_F, l_speed); // Front-Left
    control_motor(L_IA_R, L_IB_R, l_speed); // Rear-Left

    // Control right side motors
    control_motor(R_IA_F, R_IB_F, r_speed); // Front-Right
    control_motor(R_IA_R, R_IB_R, r_speed); // Rear-Right
}

// Move forward with 4WD
void forward_on(int speed)
{
    set_motor_speeds(speed, speed);
}

// Move backward with 4WD
void back_on(int speed)
{
    set_motor_speeds(-speed, -speed);
}

// Spin left on the spot with 4WD
void spin_left_on(int speed)
{
    // Left motors backward, Right motors forward
    set_motor_speeds(-speed, speed);
}

// Spin right on the spot with 4WD
void spin_right_on(int speed)
{
    // Right motors backward, Left motors forward
    set_motor_speeds(speed, -speed);
}

// Stop all 4 motors
void car_stop()
{
    set_motor_speeds(0, 0);
}

// Brake all 4 motors by applying a short reverse pulse
void car_brake(int time)
{
    // Speeds are inverted for a moment
    set_motor_speeds(-OPT_SPEED / 2, -OPT_SPEED / 2);
    delay(time);
    car_stop();
}
