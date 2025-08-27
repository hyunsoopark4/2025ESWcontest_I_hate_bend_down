#ifndef BT_COMMAND_H
#define BT_COMMAND_H

#include <Arduino.h>
#include <NeoSWSerial.h>

// 블루투스 RX/TX 핀
#define BT_RX_PIN 12
#define BT_TX_PIN 13

// 버퍼 크기
#define BUFFERSIZE 32
#define LOOP_LATENCY 50

// 명령어 정의 - 여기서만 수정하면 됨!
#define COMMAND_LIST(X) \
    X(UNKNOWN, "UNKNOWN") \
    X(OPEN, "OPN") \
    X(CLOSE, "CLS") \
    X(FORWARD, "F") \
    X(FORWARD_TORQUE, "FT") \
    X(LEFT, "L") \
    X(RIGHT, "R") \



// enum 자동 생성
enum CommandType {
#define GENERATE_ENUM(ENUM, STRING) CMD_##ENUM,
    COMMAND_LIST(GENERATE_ENUM)
#undef GENERATE_ENUM
};

void bt_init();
int bt_checkCommand();  // 수신 확인 및 명령 해석

#endif
