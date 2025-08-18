#ifndef SERVO_GRIP_H
#define SERVO_GRIP_H

#include <Arduino.h>
#include <Servo.h>

class ServoGrip {
public:
    ServoGrip(int pin, int openAngle = 0, int closeAngle = 180);
    void attach();
    void openGrip();
    void closeGrip();
    void moveServoSmoothly(int targetAngle);

private:
    int _pin;
    int _openAngle;
    int _closeAngle;
    Servo _servo;
};

#endif
