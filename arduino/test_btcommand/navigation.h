#ifndef NAVIGATION_H
#define NAVIGATION_H

// 로봇 이동 방향 정의 (4방향)
#define NORTH 1
#define EAST 2
#define SOUTH 3
#define WEST 4

// 목표물 접근 방향 정의 (8방향)
#define TARGET_NORTH 1
#define TARGET_NORTH_EAST 2
#define TARGET_EAST 3
#define TARGET_SOUTH_EAST 4
#define TARGET_SOUTH 5
#define TARGET_SOUTH_WEST 6
#define TARGET_WEST 7
#define TARGET_NORTH_WEST 8

// 현재 로봇의 위치와 방향 상태
struct RobotState {
    int x;
    int y;
    int direction;  // 1~8 방향
};

// 목표물의 정보
struct Target {
    int x;
    int y;
    int approach_direction;  // 1~8 방향
};

// 함수 선언
void init_navigation();  // 초기 위치(0,0) 설정
void set_target(int x, int y, int direction);  // 목표물 정보 설정
void navigate_to_target();  // 목표물까지 경로 계산 및 이동
void move_to_position(int x, int y);  // 특정 좌표로 이동
void adjust_direction(int target_direction);  // 목표 방향으로 회전
void return_to_origin();

// 위치 추적 관련 함수 추가
void update_position();  // 현재 위치 업데이트
void update_direction(bool turn_left);  // 회전 시 방향 업데이트
void send_current_state();  // 현재 상태(위치,방향) 전송

extern RobotState current_state;
extern Target target_info;

#endif