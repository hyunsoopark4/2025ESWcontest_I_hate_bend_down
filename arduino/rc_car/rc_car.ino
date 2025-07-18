// 서보모터
#include <NeoSWSerial.h>
#include "bt_serial.h"
#include "dc_motor.h"
#include "servo.h"
#include "ultrasonic.h"

#define CYCLE_DEGREE 5 // the degree cycled per the assigned tick

#define STRAIGHT_TICKRATE 30
// enum each_func {L_MOTOR, R_MOTOR, FORWARD, BACK, LEFT_TURN, R_TURN};

// BT
#define COUNTOF_COMMAND 2
#define COUNTOF_KEY 5
#define BUFFERSIZE 64
#define AUTO_MODE 0
#define MANUAL_MODE 1
#define MANUAL_TICKRATE

#define LOOP_LATENCY 50

// LineTracing
#define FF 0  // 십자 교차로 (S1 + S4 감지)
#define FL 1  // 왼쪽 T자 (S1 감지)
#define FR 2  // 오른쪽 T자 (S4 감지)

#define SENSOR_LEFT 7  // 왼쪽 끝
#define SENSOR_MID_L 9 // 중앙 왼쪽
#define SENSOR_MID_R 10 // 중앙 오른쪽
#define SENSOR_RIGHT 8 // 오른쪽 끝


char clientRead;
char command[COUNTOF_COMMAND][BUFFERSIZE] = {"auto", "manual"};
char sendBuffer[BUFFERSIZE];
char key[COUNTOF_KEY] = "sflrb";
int currentMode = AUTO_MODE;

NeoSWSerial mySerial(12, 13); // RX, TX

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600); // **  9600 Baud rate should be. **
    mySerial.begin(9600);

    // serial verify
    Serial.println("Serial initiated");
    mySerial.println("BT Serial initiated");

    // 서보모터
//    servo_setup();

    // DC모터
    car_stop();

    // 초음파
//    ultrasonic_setup();

    // 라인트레이싱 센서
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
}

void loop()
{
    clientRead = -1; // init client message

    if (mySerial.available())
    {                                 // only when recieved
        clientRead = mySerial.read(); // read from client

        Serial.println(clientRead);

        switch (clientRead)
        {
        case 's':
            car_stop();
            break;
        case 'f':
            forward_on();
            break;
        case 'l':
            spin_left_on();
            break;
        case 'r':
            spin_right_on();
            break;
        case 'b':
            back_on();
            break;
        }

        if (clientRead == '/' || clientRead == '!')
        {
            call_command();
        }
        return;
    }
    else
    {
        if (currentMode == AUTO_MODE)
        {
            //autopilot();
            linetracing(FL);
        }
        return;
    }
}

void linetracing(int mode)
{
    bool s1 = digitalRead(SENSOR_LEFT);   
    bool s2 = digitalRead(SENSOR_MID_L);  
    bool s3 = digitalRead(SENSOR_MID_R);  
    bool s4 = digitalRead(SENSOR_RIGHT);  

    // 1. 교차점 도달 조건
    bool is_cross = false;

//    if (mode == FF && s1 && s4)
//    {
//        is_cross = true;
//    }
//    else if (mode == FL && s1 && !s4)
//    {
//        is_cross = true;
//    }
//    else if (mode == FR && !s1 && s4)
//    {
//        is_cross = true;
//    }

    if(s1 || s4) is_cross = true;

    if (is_cross)
    {
        Serial.print("교차점 감지됨: ");
        if (mode == FF) Serial.println("FF");
        if (mode == FL) Serial.println("FL");
        if (mode == FR) Serial.println("FR");

        back_on(OPT_SPEED/2);
        delay(100);
        car_stop();

        delay(5000);

        
        return;
    }

    // 2. 라인트레이싱: 중앙 두 센서 기준
    if (s2 && !s3)
    {
        // 왼쪽으로 틀어짐 → 오른쪽 보정
        set_motor_speeds(OPT_SPEED, OPT_SPEED/2);
        Serial.println("Adjust right");
    }
    else if (!s2 && s3)
    {
        // 오른쪽으로 틀어짐 → 왼쪽 보정
        set_motor_speeds(OPT_SPEED/2, OPT_SPEED);
        Serial.println("Adjust left");
    }
    else
    {
        forward_on(OPT_SPEED);
        Serial.println("Forward");
    }
}

void autopilot()
{
    car_stop();
    Serial.println("autopilot initiated");

    servo_front();      // look forward
    if (!is_obstacle()) // no obstacle forward?
    {
        servo_front(); // init servo
        // Dont turn
        Serial.println("heading front");
        straight_auto(); // and go straight

        return;
    }

    servo_left();       // look left
    if (!is_obstacle()) // no obstacle left?
    {
        servo_front(); // init servo
        turn_left();   // then turn left
        Serial.println("heading left");
        return;
    }

    servo_front(); // init servo

    servo_right();      // look left
    if (!is_obstacle()) // no obstacle left?
    {
        servo_front(); // init servo
        turn_right();  // then turn left
        Serial.println("heading right");
        return;
    }

    // obstacle everywhere
    Serial.println("turning back");
    servo_front(); // init servo
    turn_back();   // then turn back
    // don't let go
    return;
}

void call_command()
{
    int n = 0;
    int i = 0;
    char buffer[BUFFERSIZE];
    memset(buffer, 0, sizeof(buffer)); // buffer 배열 초기화

    delay(LOOP_LATENCY);

    while (mySerial.available() && n < BUFFERSIZE - 1)
    {
        char ch = mySerial.read();

        // 명령 구분 문자 도달 시 종료 (예: 개행문자)
        if (ch == '\n' || ch == '\r')
            break;

        buffer[n++] = ch;
    }
    buffer[n] = '\0'; // null 종료

    while (i < COUNTOF_COMMAND)
    {
        if (strcmp(buffer, command[i]) == 0)
        {
            break;
        }
        i++;
    }

    switch (i)
    {
    case 0:
        switchmode(AUTO_MODE);
        break;
    case 1:
        switchmode(MANUAL_MODE);
        break;
    case 2:
        snprintf(sendBuffer, sizeof(sendBuffer), "Unknown Command: %s\n", buffer);
        // mySerial.write(sendBuffer);
        printf_chunked(mySerial, "Unknown Command: %s\n", buffer);
        Serial.write(sendBuffer);
        break;
    }

    return;
}

void switchmode(int modifiedMode)
{
    car_stop();
    currentMode = modifiedMode;
    snprintf(sendBuffer, sizeof(sendBuffer), "The mode has been set to %s.\n", command[currentMode]);
    printf_chunked(mySerial, "The mode has been set to %s.\n", command[currentMode]);
    // mySerial.write(sendBuffer);  // BT 출력 안정화
    Serial.write(sendBuffer); // USB Serial에도 동일하게 출력
}


// 장애물 살피면서 가기
void straight_auto()
{
    int scan_sequences[3][2] = {{90, 150}, {150, 30}, {30, 90}};
    int step = 0;
    int current_deg = scan_sequences[step][0];
    int target_deg = scan_sequences[step][1];

    forward_on();

    while (1)
    {
        servo_write(current_deg);
        delay(STRAIGHT_TICKRATE);

        if (!(current_deg % 15))
        { // only call is_obstacle func EVERY 15x degree, in order to save resources.
            if (is_obstacle() || mySerial.available())
            {
                return;
            }
        }

        switch (step)
        {
        case 0:
            current_deg += CYCLE_DEGREE;
            if (current_deg > target_deg)
            {
                step++;
                current_deg = scan_sequences[step][0];
                target_deg = scan_sequences[step][1];
            }
            continue;

        case 1:
            current_deg -= CYCLE_DEGREE;
            if (current_deg < target_deg)
            {
                step++;
                current_deg = scan_sequences[step][0];
                target_deg = scan_sequences[step][1];
            }
            continue;

        case 2:
            current_deg += CYCLE_DEGREE;
            if (current_deg > target_deg)
            {
                step = 0;
                current_deg = scan_sequences[step][0];
                target_deg = scan_sequences[step][1];
            }
            continue;
    
        
        default:
            break;
        }
    }
}
