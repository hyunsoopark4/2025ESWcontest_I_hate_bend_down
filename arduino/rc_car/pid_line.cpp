#include "pid_line.h"
#include "dc_motor.h"

// PID 상수(Kp, Ki, Kd) <- 적당히 튜닝해서 보정을 해야합니다. 그럼 라인 흔들림?을 더 잡을 수 있어요.
// 튜닝 방향은 GPT한테 코드 주고, 물어보면서 해야할 것 같아요.
// 실제 로봇 움직임 보면서 튜닝을 해야 해서, 우선 GPT 추천 값으로 넣어놨습니다. - 0720
float Kp = 40.0;        //Error (오차 크기) 가중치
float Ki = 0.4;         //Integral (누적) 가중치
float Kd = 30.0;        //derivative (보정수준) 가중치

int base_speed = 100;

int last_error = 0;
int integral = 0;

const int weights[4] = {-3, -1, 1, 3};

// 내부 상태
int node_count = 0;
bool crossed = false;

void pid_line_setup()
{
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MID_L, INPUT);
    pinMode(SENSOR_MID_R, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
}

void pid_linetrace(int target_node)
{
    int sensor[4];
    sensor[0] = digitalRead(SENSOR_LEFT);   // S1
    sensor[1] = digitalRead(SENSOR_MID_L);  // S2
    sensor[2] = digitalRead(SENSOR_MID_R);  // S3
    sensor[3] = digitalRead(SENSOR_RIGHT);  // S4

    // === 교차점 감지 ===
    bool is_cross = sensor[0] || sensor[3]; // S1 또는 S4 감지

    if (is_cross && !crossed)
    {

        last_error = 0;
        integral = 0;

        crossed = true;
        node_count++;

        Serial.print("교차점 통과: ");
        Serial.print(node_count);
        Serial.print("/");
        Serial.println(target_node);

        if (node_count >= target_node)
        {
            car_stop();
            Serial.println("목표 교차점 도달. 정지.");
            return;
        }

        // 중복 감지 방지를 위해 살짝 밀기
        forward_on(220);
        delay(250);
    }

    if (!is_cross) {
        crossed = false;
    }

    // === PID 제어 ===
    int weighted_sum = 0;
    int active_count = 0;

    /*
    for (int i = 0; i < 4; i++) {
        if (sensor[i]) {
            weighted_sum += weights[i];
            active_count++;
        }
    }
    */

    if (sensor[1]) weighted_sum += -1;
    if (sensor[2]) weighted_sum += 1;

    //int error = (active_count > 0) ? (weighted_sum / active_count) : last_error;
    int error = (active_count > 0) ? weighted_sum : last_error;
    //integral += abs(error);
    integral += abs(error);
    //integral = constrain(integral, -100, 100);
    integral = constrain(integral, 0, 100);
    int derivative = error - last_error;
    last_error = error;

    //int output = (Kp * error) + (Ki * integral) + (Kd * derivative);
    int output = ((Kp * error) + (Kd * derivative)) * Ki *(100 - integral);

    int left_speed = constrain(base_speed + output, 0, 255);
    int right_speed = constrain(base_speed - output, 0, 255);

    set_motor_speeds(left_speed,right_speed);
}

