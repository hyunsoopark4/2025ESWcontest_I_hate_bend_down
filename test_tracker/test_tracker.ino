#include <Arduino.h>

#define SENSOR_LEFT 7  // 왼쪽 끝
#define SENSOR_MID_L 9 // 중앙 왼쪽
#define SENSOR_MID_R 10 // 중앙 오른쪽
#define SENSOR_RIGHT 8 // 오른쪽 끝

void setup()
{
    Serial.begin(9600); // **  9600 Baud rate should be. **
    Serial.println("Serial initiated");

    // 라인트레이싱 센서
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
}

void loop()
{
    int sensor[4];
    sensor[0] = digitalRead(SENSOR_LEFT);   // S1
    sensor[1] = digitalRead(SENSOR_MID_L);  // S2
    sensor[2] = digitalRead(SENSOR_MID_R);  // S3
    sensor[3] = digitalRead(SENSOR_RIGHT);  // S4

    for(int i=0;i<4;i++)
        Serial.println(String(sensor[i])+"");

    delay(1000);
}

