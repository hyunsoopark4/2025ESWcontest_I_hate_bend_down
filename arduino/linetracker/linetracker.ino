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


// =============================================================
//                  핀 번호 및 상수 정의
// =============================================================

// 모터 제어 핀 (L298N, TB6612 등 모터 드라이버에 맞게 수정)
#define L_MOTOR_PWM 5
#define L_MOTOR_IN1 7
#define L_MOTOR_IN2 8
#define R_MOTOR_PWM 6
#define R_MOTOR_IN1 9
#define R_MOTOR_IN2 10

// 센서 핀 (사용하는 아두이노 핀 번호로 수정)
#define FRONT_LEFT_SENS   A0
#define FRONT_RIGHT_SENS  A1
#define MIDDLE_LEFT_SENS  A2
#define MIDDLE_RIGHT_SENS A3
#define REAR_LEFT_SENS    A4
#define REAR_RIGHT_SENS   A5

// 주행 모드 정의 (간소화)
#define FF 0 // 십자(十字) 교차로 감지 모드
#define FL 1 // 전방-좌측(T자) 교차로 감지 모드
#define FR 2 // 전방-우측(T자) 교차로 감지 모드

// =============================================================
//                      모터 제어 함수
// =============================================================

void controlMotors(int leftSpeed, int rightSpeed)
{
    // 왼쪽 모터 제어 (양수: 정회전, 음수: 역회전)
    if (leftSpeed > 0) {
        digitalWrite(L_MOTOR_IN1, HIGH);
        digitalWrite(L_MOTOR_IN2, LOW);
        analogWrite(L_MOTOR_PWM, leftSpeed);
    } else if (leftSpeed < 0) {
        digitalWrite(L_MOTOR_IN1, LOW);
        digitalWrite(L_MOTOR_IN2, HIGH);
        analogWrite(L_MOTOR_PWM, -leftSpeed);
    } else {
        digitalWrite(L_MOTOR_IN1, LOW);
        digitalWrite(L_MOTOR_IN2, LOW);
        analogWrite(L_MOTOR_PWM, 0);
    }

    // 오른쪽 모터 제어 (양수: 정회전, 음수: 역회전)
    if (rightSpeed > 0) {
        digitalWrite(R_MOTOR_IN1, HIGH);
        digitalWrite(R_MOTOR_IN2, LOW);
        analogWrite(R_MOTOR_PWM, rightSpeed);
    } else if (rightSpeed < 0) {
        digitalWrite(R_MOTOR_IN1, LOW);
        digitalWrite(R_MOTOR_IN2, HIGH);
        analogWrite(R_MOTOR_PWM, -rightSpeed);
    } else {
        digitalWrite(R_MOTOR_IN1, LOW);
        digitalWrite(R_MOTOR_IN2, LOW);
        analogWrite(R_MOTOR_PWM, 0);
    }
}

// =============================================================
//                라인 트레이싱 및 교차로 감지 함수
// =============================================================

void line(int mode, int spd, int brakeTime)
{
    Serial.print("line() Start. Mode: "); Serial.println(mode);

    while (true)
    {
        // 1. 모든 센서 값 읽기 (검은색 라인 = HIGH, 흰색 = LOW 라고 가정)
        bool frontLeftOnLine  = (digitalRead(FRONT_LEFT_SENS) == HIGH);
        bool frontRightOnLine = (digitalRead(FRONT_RIGHT_SENS) == HIGH);
        bool middleLeftOnLine = (digitalRead(MIDDLE_LEFT_SENS) == HIGH);
        bool middleRightOnLine= (digitalRead(MIDDLE_RIGHT_SENS) == HIGH);

        // 2. 종료 조건 확인 (교차로 감지)
        if (mode == FF) { // 십자 교차로 감지
            if (middleLeftOnLine && middleRightOnLine) {
                Serial.println("Cross detected (FF)! Exiting loop.");
                break;
            }
        }
        else if (mode == FL) { // 좌측 T자 교차로 감지
            if (middleLeftOnLine && !middleRightOnLine) {
                Serial.println("Left T-Junction detected (FL)! Exiting loop.");
                break;
            }
        }
        else if (mode == FR) { // 우측 T자 교차로 감지
            if (!middleLeftOnLine && middleRightOnLine) {
                Serial.println("Right T-Junction detected (FR)! Exiting loop.");
                break;
            }
        }

        // 3. 라인 트레이싱 로직 (주행 보정)
        if (frontLeftOnLine && !frontRightOnLine) {
            controlMotors(spd * 0.3, spd);
        }
        else if (!frontLeftOnLine && frontRightOnLine) {
            controlMotors(spd, spd * 0.3);
        }
        else {
            controlMotors(spd, spd);
        }
    }

    // 4. 정지 로직
    if (brakeTime > 0) {
        controlMotors(-spd, -spd);
        delay(brakeTime);
    }

    controlMotors(0, 0);
    Serial.println("line() function finished.");
}

// =============================================================
//                      기본 설정 및 루프
// =============================================================

void setup()
{
    pinMode(L_MOTOR_PWM, OUTPUT);
    pinMode(L_MOTOR_IN1, OUTPUT);
    pinMode(L_MOTOR_IN2, OUTPUT);
    pinMode(R_MOTOR_PWM, OUTPUT);
    pinMode(R_MOTOR_IN1, OUTPUT);
    pinMode(R_MOTOR_IN2, OUTPUT);

    pinMode(FRONT_LEFT_SENS, INPUT);
    pinMode(FRONT_RIGHT_SENS, INPUT);
    pinMode(MIDDLE_LEFT_SENS, INPUT);
    pinMode(MIDDLE_RIGHT_SENS, INPUT);
    pinMode(REAR_LEFT_SENS, INPUT);

    Serial.begin(9600);
    while (!Serial);
    Serial.println("Arduino Line Tracer Ready");
}

void loop()
{
    // --- 테스트 시나리오 (호출 방식 변경) ---
    Serial.println("\n[Test 1] 십자 교차로(FF)까지 주행합니다. (3초 후 시작)");
    delay(3000);
    line(FF, 150, 50); 

    delay(2000);

    Serial.println("\n[Test 2] 좌측 교차로(FL)까지 주행합니다. (3초 후 시작)");
    delay(3000);
    line(FL, 150, 50);

    delay(2000);

    Serial.println("\n[Test 3] 우측 교차로(FR)까지 주행합니다. (3초 후 시작)");
    delay(3000);
    line(FR, 150, 50);

    delay(5000);
}
