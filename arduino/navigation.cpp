#include "navigation.h"
#include "line_trace.h"
#include "dc_motor.h"
#include "bt_command.h"  // mySerial 사용을 위해 추가

extern NeoSWSerial mySerial;  // bt_command.cpp에 정의된 mySerial 사용

RobotState current_state;
Target target_info;

// 방향 전환 시 현재 방향 업데이트
void update_direction(bool turn_left) {
    if(turn_left) {
        // 반시계 방향 회전 (NORTH->WEST->SOUTH->EAST)
        current_state.direction = ((current_state.direction - 2 + 4) % 4) + 1;
    } else {
        // 시계 방향 회전 (NORTH->EAST->SOUTH->WEST)
        current_state.direction = (current_state.direction % 4) + 1;
    }
}

// 전진 시 현재 위치 업데이트
void update_position() {
    switch(current_state.direction) {
        case NORTH:
            current_state.y++; break;
        case SOUTH:
            current_state.y--; break;
        case EAST:
            current_state.x++; break;
        case WEST:
            current_state.x--; break;
    }
}

void init_navigation() {
    current_state.x = 0;
    current_state.y = 0;
    current_state.direction = NORTH;
}

void set_target(int x, int y, int direction) {
    target_info.x = x;
    target_info.y = y;
    target_info.approach_direction = direction;
}

void navigate_to_target() {
    // 1. 목표 근처까지 이동
    int approach_x = target_info.x;
    int approach_y = target_info.y;
    
    // 접근 방향에 따라 접근 위치 조정
    switch(target_info.approach_direction) {
        case NORTH: approach_y--; break;
        case SOUTH: approach_y++; break;
        case EAST: approach_x--; break;
        case WEST: approach_x++; break;
    }
    
    // 2. 접근 위치로 이동
    move_to_position(approach_x, approach_y);
    
    // 3. 목표물을 향해 방향 전환
    adjust_direction(target_info.approach_direction);
    
    // 4. 목표물 접근 (라인트레이싱으로)
    line_trace_torque();  // 토크 모드로 접근
    // TODO: grab_target() 함수 호출 필요
    
    // 5. 원점으로 복귀
    return_to_origin();
}

void move_to_position(int target_x, int target_y) {
    // 좌표 이동 시작 메시지
    mySerial.println("Moving to: (" + String(target_x) + "," + String(target_y) + ")");
    
    // 1단계: Y축 이동 (세로 방향)
    int dy = target_y - current_state.y;
    if(dy != 0) {
        // Y축 방향 결정 및 회전
        int target_direction = (dy > 0) ? NORTH : SOUTH;
        if(current_state.direction != target_direction) {
            adjust_direction(target_direction);
        }
        
        // Y축 이동
        while(current_state.y != target_y) {
            line_trace();
            update_position();
            send_current_state();
        }
    }
    
    // 2단계: X축 이동 (가로 방향)
    int dx = target_x - current_state.x;
    if(dx != 0) {
        // X축 방향 결정 및 회전
        int target_direction = (dx > 0) ? EAST : WEST;
        if(current_state.direction != target_direction) {
            adjust_direction(target_direction);
        }
        
        // X축 이동
        while(current_state.x != target_x) {
            line_trace();
            update_position();
            send_current_state();
        }
    }
    
    // 3단계: 최종 북쪽 방향 정렬
    if(current_state.direction != NORTH) {
        adjust_direction(NORTH);
        send_current_state();
    }
    
    mySerial.println("Position reached!");
}

void send_current_state() {
    String direction_str;
    switch(current_state.direction) {
        case NORTH: direction_str = "N"; break;
        case EAST: direction_str = "E"; break;
        case SOUTH: direction_str = "S"; break;
        case WEST: direction_str = "W"; break;
    }
    
    String state_msg = String(current_state.x) + "," 
                    + String(current_state.y) + "," 
                    + direction_str;
    
    mySerial.println(state_msg);  // 블루투스로 전송
    Serial.println(state_msg);    // 시리얼 모니터로도 전송 (디버깅용)
}

void adjust_direction(int target_direction) {
    while(current_state.direction != target_direction) {
        int diff = target_direction - current_state.direction;
        if((diff + 4) % 4 <= 2) {  // 시계방향이 더 가까움
            turn_right();
            update_direction(false);
        } else {  // 반시계방향이 더 가까움
            turn_left();
            update_direction(true);
        }
    }
}

void return_to_origin() {
    // 원점으로 이동
    move_to_position(0, 0);
    
    // 북쪽 방향으로 정렬
    adjust_direction(NORTH);
}