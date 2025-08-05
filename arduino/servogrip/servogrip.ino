#include <Servo.h>

// 핀 정의
const int POTENTIOMETER_PIN = A0;  // 가변저항 핀 (아날로그 입력)
const int SERVO_PIN = 9;           // 서보모터 핀 (PWM 출력)

// 서보모터 객체 생성
Servo gripServo;

// 변수 정의
int potValue = 0;      // 가변저항 읽기 값 (0-1023)
int servoAngle = 0;    // 서보모터 각도 (0-180)

void setup() {
  // 시리얼 통신 초기화
  Serial.begin(9600);
  
  // 서보모터 핀 연결
  gripServo.attach(SERVO_PIN);
  
  // 가변저항 핀 초기화 (INPUT은 기본값이므로 생략 가능)
  pinMode(POTENTIOMETER_PIN, INPUT);
  
  Serial.println("Servo Grip Controller Started");
  Serial.println("Use potentiometer to control servo position");
}

void loop() {
  // 가변저항 값 읽기 (0-1023)
  potValue = analogRead(POTENTIOMETER_PIN);
  
  // 가변저항 값을 서보모터 각도로 변환 (0-1023 -> 0-180)
  servoAngle = map(potValue, 0, 1023, 0, 180);
  
  // 서보모터 위치 설정
  gripServo.write(servoAngle);
  
  // 시리얼 모니터에 현재 값 출력
  Serial.print("Potentiometer Value: ");
  Serial.print(potValue);
  Serial.print(" -> Servo Angle: ");
  Serial.println(servoAngle);
  
  // 짧은 지연 (서보모터가 움직일 시간 제공)
  delay(50);
}

// 서보모터를 특정 각도로 부드럽게 이동시키는 함수
void moveServoSmoothly(int targetAngle) {
  int currentAngle = gripServo.read();
  
  if (currentAngle < targetAngle) {
    // 목표 각도가 현재보다 클 때
    for (int angle = currentAngle; angle <= targetAngle; angle++) {
      gripServo.write(angle);
      delay(15);
    }
  } else {
    // 목표 각도가 현재보다 작을 때
    for (int angle = currentAngle; angle >= targetAngle; angle--) {
      gripServo.write(angle);
      delay(15);
    }
  }
}

// 그립 열기 함수
void openGrip() {
  moveServoSmoothly(0);
  Serial.println("Grip Opened");
}

// 그립 닫기 함수
void closeGrip() {
  moveServoSmoothly(180);
  Serial.println("Grip Closed");
}
