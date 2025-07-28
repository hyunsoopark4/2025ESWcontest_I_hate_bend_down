// 초음파센서
#define TRIG_PIN 8
#define ECHO_PIN 9
#define OBS 40 // Distance of obstacles [Cm]
#include <Arduino.h>


int get_ultrasonic_cm()
{
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    unsigned long duration = pulseIn(ECHO_PIN, HIGH, 11765);
    int distanceCM = duration * 0.017;

    return distanceCM;
}

bool is_obstacle()
{
    int distanceCM = get_ultrasonic_cm();

    if (2 <= distanceCM && distanceCM <= OBS)
    {
        return 1;
    }
    return 0;
}

int ultrasonic_setup()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}