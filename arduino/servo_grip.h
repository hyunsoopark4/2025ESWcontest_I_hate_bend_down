#ifndef SERVO_GRIP_H
#define SERVO_GRIP_H

#include <Arduino.h>
#include <Servo.h>

#define SMOOTH_DELAY 30 // 서보 모터 이동 시 딜레이 (밀리초)

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
