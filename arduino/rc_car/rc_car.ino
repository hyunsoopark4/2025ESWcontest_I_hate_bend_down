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

char clientRead;
char command[COUNTOF_COMMAND][BUFFERSIZE] = {"auto", "manual"};
char sendBuffer[BUFFERSIZE];
char key[COUNTOF_KEY] = "sflrb";
int currentMode = MANUAL_MODE;

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
    servo_setup();

    // DC모터
    car_stop();

    // 초음파
    ultrasonic_setup();
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
            autopilot();
        }
        return;
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

