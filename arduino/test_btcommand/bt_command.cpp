#include "bt_command.h"
#include "navigation.h"
#include "line_trace.h" // line_trace, turn_left, turn_right 함수 사용을 위해 추가
#include "pid.h"        // LinePID 클래스 사용을 위해 추가

NeoSWSerial mySerial(BT_RX_PIN, BT_TX_PIN);

// 명령어 문자열 배열 자동 생성
static const char *command[] = {
#define GENERATE_STRING(ENUM, STRING) STRING,
    COMMAND_LIST(GENERATE_STRING)
#undef GENERATE_STRING
};

static char buffer[BUFFERSIZE];

void bt_init()
{
    mySerial.begin(9600);
}

void parse_coordinates(const char *input, int *x, int *y)
{
    if (sscanf(input, "(%d,%d)", x, y) != 2)
    {
        *x = -1;
        *y = -1;
    }
}

void send_movement_msg(const char *msg)
{
    mySerial.println(msg);
    Serial.println(msg);
    delay(500); // 각 동작 후 500ms 딜레이
}

int bt_checkCommand()
{
    if (!mySerial.available())
        return CMD_UNKNOWN;

    int n = 0;
    memset(buffer, 0, sizeof(buffer));

    delay(LOOP_LATENCY);

    while (mySerial.available() && n < BUFFERSIZE - 1)
    {
        char ch = mySerial.read();
        if (ch == '\n' || ch == '\r')
            break;
        buffer[n++] = ch;
    }
    buffer[n] = '\0';

    Serial.println(buffer);

    // 좌표 입력 확인
    if (buffer[0] == '(')
    {
        // 토크 모드 확인 ('T' 접미사)
        bool isTorqueMode = false;
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == 'T')
        {
            isTorqueMode = true;
            buffer[len - 1] = '\0'; // T 제거하여 좌표만 파싱
        }

        int x, y;
        parse_coordinates(buffer, &x, &y);
        if (x >= 0 && y >= 0)
        {
            String msg = "Moving to coordinate: (" + String(x) + "," + String(y) + ")";
            if (isTorqueMode)
                msg += " [Torque Mode]";
            send_movement_msg(msg.c_str());

            // Y축 먼저 이동 (북/남)
            while (current_state.y != y)
            {
                if (current_state.y < y)
                { // 북쪽으로 이동
                    if (current_state.direction != NORTH)
                    {
                        send_movement_msg("Rotating to NORTH");
                        if (current_state.direction == EAST)
                        {
                            isTorqueMode ? torque_turn_left() : turn_left();
                            update_direction(true);
                        }
                        else if (current_state.direction == WEST)
                        {
                            isTorqueMode ? torque_turn_right() : turn_right();
                            update_direction(false);
                        }
                        else if (current_state.direction == SOUTH)
                        {
                            isTorqueMode ? torque_turn_right() : turn_right();
                            update_direction(false);
                            isTorqueMode ? line_trace_torque() : line_pid.pid_linetrace();
                            isTorqueMode ? torque_turn_right() : turn_right();
                            update_direction(false);
                        }
                    }
                    send_movement_msg("Moving NORTH");
                    isTorqueMode ? line_trace_torque() : line_pid.pid_linetrace();
                    update_position();
                }
                else
                { // 남쪽으로 이동
                    if (current_state.direction != SOUTH)
                    {
                        send_movement_msg("Rotating to SOUTH");
                        if (current_state.direction == EAST)
                        {
                            isTorqueMode ? torque_turn_right() : turn_right();
                            update_direction(false);
                        }
                        else if (current_state.direction == WEST)
                        {
                            isTorqueMode ? torque_turn_left() : turn_left();
                            update_direction(true);
                        }
                        else if (current_state.direction == NORTH)
                        {
                            isTorqueMode ? torque_turn_right() : turn_right();
                            update_direction(false);
                            isTorqueMode ? line_trace_torque() : line_pid.pid_linetrace();
                            isTorqueMode ? torque_turn_right() : turn_right();
                            update_direction(false);
                        }
                    }
                    send_movement_msg("Moving SOUTH");
                    isTorqueMode ? line_trace_torque() : line_pid.pid_linetrace();
                    update_position();
                }
            }

            // X축 이동 (동/서)
            while (current_state.x != x)
            {
                if (current_state.x < x)
                { // 동쪽으로 이동
                    if (current_state.direction != EAST)
                    {
                        send_movement_msg("Rotating to EAST");
                        isTorqueMode ? torque_turn_right() : turn_right();
                        update_direction(false);
                    }
                    send_movement_msg("Moving EAST");
                    isTorqueMode ? line_trace_torque() : line_pid.pid_linetrace();
                    ;
                    update_position();
                }
                else
                { // 서쪽으로 이동
                    if (current_state.direction != WEST)
                    {
                        send_movement_msg("Rotating to WEST");
                        isTorqueMode ? torque_turn_left() : turn_left();
                        update_direction(true);
                    }
                    send_movement_msg("Moving WEST");
                    isTorqueMode ? line_trace_torque() : line_pid.pid_linetrace();
                    update_position();
                }
            }

            // 마지막으로 북쪽 방향으로 정렬
            if (current_state.direction != NORTH)
            {
                send_movement_msg("Final alignment to NORTH");
                if (current_state.direction == SOUTH)
                {
                    isTorqueMode ? torque_turn_right() : turn_right();
                    update_direction(false);
                    isTorqueMode ? line_trace_torque() : line_pid.pid_linetrace();
                    isTorqueMode ? torque_turn_right() : turn_right();
                    update_direction(false);
                }
                else if (current_state.direction == EAST)
                {
                    isTorqueMode ? torque_turn_left() : turn_left();
                    update_direction(true);
                }
                else if (current_state.direction == WEST)
                {
                    isTorqueMode ? torque_turn_right() : turn_right();
                    update_direction(false);
                }
            }

            send_movement_msg("Reached target position!");
            return CMD_MOVE_TO;
        }
    }

    // 기존 명령어 처리
    for (int i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(buffer, command[i]) == 0)
        {
            return i;
        }
    }

    return CMD_UNKNOWN;
}
