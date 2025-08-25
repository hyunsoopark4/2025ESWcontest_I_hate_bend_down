#include "bt_command.h"

NeoSWSerial mySerial(BT_RX_PIN, BT_TX_PIN);

// 명령어 문자열 배열 자동 생성
static const char* command[] = {
#define GENERATE_STRING(ENUM, STRING) STRING,
    COMMAND_LIST(GENERATE_STRING)
#undef GENERATE_STRING
};

// 명령어 개수 자동 계산
static const int COMMAND_COUNT = sizeof(command) / sizeof(command[0]); // UNKNOWN 제외

char buffer[BUFFERSIZE];

void bt_init() {
    mySerial.begin(9600);
}

int bt_checkCommand() {
    if (!mySerial.available()) return CMD_UNKNOWN;

    int n = 0;
    memset(buffer, 0, sizeof(buffer));

    delay(LOOP_LATENCY);

    while (mySerial.available() && n < BUFFERSIZE - 1) {
        char ch = mySerial.read();
        if (ch == '\n' || ch == '\r') break;
        buffer[n++] = ch;
    }
    buffer[n] = '\0';

    Serial.println(buffer);

    // 자동으로 모든 명령어 검사
    for (int i = 1; i < COMMAND_COUNT; i++) {
        if (strcmp(buffer, command[i]) == 0) {
            return i; // enum 값과 배열 인덱스가 일치
        }
    }

    return CMD_UNKNOWN;
}
