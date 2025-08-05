// 서보모터
#include "bt_serial.h"
#include "dc_motor.h"
#include "linetracing.h"

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
char sendBuffer[BUFFERSIZE];

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600); // **  9600 Baud rate should be. **
    BTserial.begin(9600);

    // USB 시리얼과 블루투스 시리얼에 각각 초기화 메시지 전송
    Serial.println("RC Car System Initialized.");
    printf_chunked(BTserial, "RC Car System Initialized.\n");
    // DC모터
    car_stop();

    // 적외선 센서 초기화
    linetrace_setup();
}

void loop()
{
    move_to_node(1);     // 함수 호출 시 1 ~ N까지 교차점 개수 설정할 수 있습니다. 
    delay(1000);
}

// 한 칸(노드)씩 이동하며 최종 목적 노드까지 이동하는 함수
void move_to_node(int target)
{
    node_count = 0;

    while (node_count < target) {
        run_linetracing(target);
    }
    
    car_brake(120);
}
