#include "bt_command.h"

NeoSWSerial mySerial(BT_RX_PIN, BT_TX_PIN);

static const char* command[] = {
    "OPEN",
    "CLOSE",
    "UNKNOWN"
};

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

    if (strcmp(buffer, command[0]) == 0) return CMD_OPEN;
    if (strcmp(buffer, command[1]) == 0) return CMD_CLOSE;

    return CMD_UNKNOWN;
}
