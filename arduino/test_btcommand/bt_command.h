#ifndef BT_COMMAND_H
#define BT_COMMAND_H

#include <Arduino.h>
#include <NeoSWSerial.h>

// 블루투스 RX/TX 핀
#define BT_RX_PIN 12
#define BT_TX_PIN 13

// 버퍼 크기
#define BUFFERSIZE 32
#define LOOP_LATENCY 20  // 20ms

// 명령어 정의
#define COMMAND_LIST(X) \
    X(UNKNOWN, "UNKNOWN") \
    X(OPEN, "OPN") \
    X(CLOSE, "CLS") \
    X(FORWARD, "F") \
    X(FORWARD_TORQUE, "FT") \
    X(LEFT, "L") \
    X(RIGHT, "R") \
    X(LEFT_TURBO, "LT") \
    X(RIGHT_TURBO, "RT") \
    X(BACKWARD, "B") \
    X(STATUS, "S") \
    X(MOVE_TO, "MOVE")

// 명령어 enum 생성
#define GENERATE_ENUM(ENUM, STRING) CMD_##ENUM,
enum CommandEnum {
    COMMAND_LIST(GENERATE_ENUM)
    COMMAND_COUNT
};
#undef GENERATE_ENUM

void bt_init();
int bt_checkCommand();

extern NeoSWSerial mySerial;

#endif
