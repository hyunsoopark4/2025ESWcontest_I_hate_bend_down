//inital
/*
왼쪽 앞 센서가 감지된 경우 = 차체가 오른쪽으로 틀어진 경우
왼쪽 모터 속도 감속
오른쪽 모터 속도 가속

오른쪽 앞 센서가 감지된 경우 = 차체가 왼쪽으로 틀어진 경우
왼쪽 모터 속도 가속
오른쪽 모터 속도 감속

양쪽 앞 센서가 모두 감지된 경우 = 한 칸 도달
라인에 맞춰 앞으로 밀어주기
*/


void controlMotors(int leftSpeed, int rightSpeed)
{
    // 왼쪽 모터 제어
    if ( leftSpeed > 0 )
    {
        digitalWrite(L_MOTOR_IN1, HIGH);
        digitalWrite(L_MOTOR_IN2, LOW);
        analogWrite(L_MOTOR_PWM, leftSpeed);
    }
    else if ( leftSpeed < 0 )
    {
        digitalWrite(L_MOTOR_IN1, LOW);
        digitalWrite(L_MOTOR_IN2, HIGH);
        analogWrite(L_MOTOR_PWM, -leftSpeed);
    }
    else
    {
        digitalWrite(L_MOTOR_IN1, LOW);
        digitalWrite(L_MOTOR_IN2, LOW);
        analogWrite(L_MOTOR_PWM, 0);
    }

    // 오른쪽 모터 제어
    if ( rightSpeed > 0 )
    {
        digitalWrite(R_MOTOR_IN1, HIGH);
        digitalWrite(R_MOTOR_IN2, LOW);
        analogWrite(R_MOTOR_PWM, rightSpeed);
    }
    else if ( rightSpeed < 0 )
    {
        digitalWrite(R_MOTOR_IN1, LOW);
        digitalWrite(R_MOTOR_IN2, HIGH);
        analogWrite(R_MOTOR_PWM, -rightSpeed);
    }
    else
    {
        digitalWrite(R_MOTOR_IN1, LOW);
        digitalWrite(R_MOTOR_IN2, LOW);
        analogWrite(R_MOTOR_PWM, 0);
    }
}

/**
 * @brief 라인을 따라 주행하다가 특정 조건을 만족하면 정지합니다.
 * @param mode 주행 모드. 현재는 MODE_FF만 지원.
 * @param spd 주행 속도 (0 ~ 255).
 * @param brakeTime 정지 후 브레이크를 잡을 시간 (ms). 0이면 즉시 정지.
 */
void line(int mode, int spd, int brakeTime)
{
    Serial.println("line() function started.");
    Serial.print("Mode: "); Serial.print(mode);
    Serial.print(", Speed: "); Serial.print(spd);
    Serial.print(", Brake Time: "); Serial.println(brakeTime);

    while ( true )
    {
        // 1. 센서 값 읽기 (검은색 라인 = LOW, 흰색 = HIGH 라고 가정)
        bool frontLeftOnLine = (digitalRead(FRONT_LEFT_SENS) == LOW);
        bool frontRightOnLine = (digitalRead(FRONT_RIGHT_SENS) == LOW);

        // 2. 종료 조건 확인 (교차로 감지)
        if ( mode == MODE_FF )
        {/
            // 양쪽 전방 센서가 모두 라인을 감지하면 교차로로 판단하고 루프 탈출
            if ( frontLeftOnLine && frontRightOnLine )
            {
                Serial.println("Cross detected! Exiting loop.");
                break;
            }
        }

        // 3. 라인 트레이싱 로직 (P제어와 유사)
        if ( frontLeftOnLine && !frontRightOnLine )
        {
            // 오른쪽으로 치우침 -> 왼쪽으로 회전
            // 왼쪽 바퀴 속도를 줄여 왼쪽으로 더 많이 돌게 함
            controlMotors(spd * 0.5, spd);
        }
        else if ( !frontLeftOnLine && frontRightOnLine )
        {
            // 왼쪽으로 치우침 -> 오른쪽으로 회전
            // 오른쪽 바퀴 속도를 줄여 오른쪽으로 더 많이 돌게 함
            controlMotors(spd, spd * 0.5);
        }
        else
        {
            // 중앙에 있거나, 라인을 잠시 놓쳤을 경우 -> 직진
            controlMotors(spd, spd);
        }
    }

    // 4. 정지 로직
    if ( brakeTime > 0 )
    {
        // 지정된 시간만큼 모터를 역회전시켜 급정지 (Active Brake)
        controlMotors(-spd, -spd);
        delay(brakeTime);
    }

    // 모터 완전 정지
    controlMotors(0, 0);
    Serial.println("line() function finished.");
}

void setup()
{
    // 모터 제어 핀들을 출력으로 설정
    pinMode(L_MOTOR_PWM, OUTPUT);
    pinMode(L_MOTOR_IN1, OUTPUT);
    pinMode(L_MOTOR_IN2, OUTPUT);
    pinMode(R_MOTOR_PWM, OUTPUT);
    pinMode(R_MOTOR_IN1, OUTPUT);
    pinMode(R_MOTOR_IN2, OUTPUT);

    // 센서 핀들을 입력으로 설정
    pinMode(FRONT_LEFT_SENS, INPUT);
    pinMode(FRONT_RIGHT_SENS, INPUT);
    pinMode(REAR_LEFT_SENS, INPUT);
    pinMode(REAR_RIGHT_SENS, INPUT);

    // 시리얼 통신 시작 (디버깅용)
    Serial.begin(9600);
    Serial.println("Arduino Line Tracer Ready");
}

void loop()
{
    Serial.println("\nStarting line following task in 3 seconds...");
    delay(3000);

    // 예시: MODE_FF(교차로 감지) 모드로, 150의 속도로, 브레이크 없이 주행
    line(MODE_FF, 150, 0);

    Serial.println("Task complete. Waiting for 5 seconds before restarting.");
    delay(5000);
}
