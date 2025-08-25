#include "servo_grip.h"

ServoGrip::ServoGrip(int pin, int openAngle, int closeAngle)
    : _pin(pin), _openAngle(openAngle), _closeAngle(closeAngle) {}

void ServoGrip::attach() {
    _servo.attach(_pin);
    openGrip(); // 초기 상태는 열림
}

void ServoGrip::openGrip() {
    moveServoSmoothly(_openAngle);
    Serial.println("Grip Opened");
}

void ServoGrip::closeGrip() {
    moveServoSmoothly(_closeAngle);
    Serial.println("Grip Closed");
}

void ServoGrip::moveServoSmoothly(int targetAngle) {
    int currentAngle = _servo.read();

    if (currentAngle < targetAngle) {
        for (int angle = currentAngle; angle <= targetAngle; angle++) {
            _servo.write(angle);
            delay(15);
        }
    } else {
        for (int angle = currentAngle; angle >= targetAngle; angle--) {
            _servo.write(angle);
            delay(15);
        }
    }
}
