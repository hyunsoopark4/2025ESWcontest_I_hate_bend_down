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

// 명령어 목록
enum CommandType {
    CMD_OPEN,
    CMD_CLOSE,
    CMD_UNKNOWN
};

void bt_init();
int bt_checkCommand();  // 수신 확인 및 명령 해석

#endif
