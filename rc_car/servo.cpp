#define SERVO_PIN 10 // D10
#define SERVO_ON 125 // servo motor on time : 180deg / 0.250 sec
#define LEFT_DEGREE 135
#define FRONT_DEGREE 90
#define RIGHT_DEGREE 45
#include <Arduino.h>
#include <Servo.h>

Servo myservo; // create servo object to control a servo

// 서보모터
void servo_setup()
{
    myservo.attach(SERVO_PIN);   // attaches the servo
}

void servo_front()
{
    myservo.write(FRONT_DEGREE); // front position
    delay(SERVO_ON);
}
void servo_left()
{
    myservo.write(LEFT_DEGREE); // left position
    delay(SERVO_ON);
}
void servo_right()
{
    myservo.write(RIGHT_DEGREE); // right position
    delay(SERVO_ON);             // double because shift from 180 degree to 0 degree
}
void servo_write(int degree)
{
    myservo.write(degree); // right position
}