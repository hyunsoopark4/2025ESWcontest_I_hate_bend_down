#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

MPU6050 mpu;

// DMP 제어 및 상태 변수
bool dmpReady = false;
uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];

// 자세(각도) 계산을 위한 변수
Quaternion q;           // [w, x, y, z] 쿼터니언 컨테이너
VectorFloat gravity;    // [x, y, z] 중력 벡터
float ypr[3];           // [yaw, pitch, roll] 요, 피치, 롤 컨테이너

// 인터럽트 감지 플래그
volatile bool mpuInterrupt = false;
void dmpDataReady() {
  mpuInterrupt = true;
}

void setup() {
  Wire.begin();
  Serial.begin(115200);

  // MPU6050 초기화
  mpu.initialize();
  devStatus = mpu.dmpInitialize();

  // DMP 초기화 성공 시, 자이로 오프셋 보정(캘리브레이션) 진행
  if (devStatus == 0) {
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // 인터럽트 설정
    attachInterrupt(digitalPinToInterrupt(2), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // 에러 코드 출력
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

void loop() {
  if (!dmpReady) return; // DMP가 준비되지 않았으면 아무것도 하지 않음

  // 인터럽트가 발생할 때까지 기다리거나, FIFO 버퍼가 꽉 찼으면 처리
  while (!mpuInterrupt && fifoCount < packetSize) {
    // 여기서 다른 작업을 수행할 수 있습니다.
  }

  // 인터럽트 플래그 리셋 및 FIFO 상태 확인
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();
  fifoCount = mpu.getFIFOCount();

  // FIFO 오버플로우 확인
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));
  } else if (mpuIntStatus & 0x02) {
    // FIFO 버퍼에서 패킷이 준비될 때까지 기다림
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // FIFO에서 새로운 패킷 읽기
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    fifoCount -= packetSize;

    // Yaw, Pitch, Roll 값 계산
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    // 결과 출력 (라디안 -> 각도 변환)
    Serial.print("ypr\t");
    Serial.print(ypr[0] * 180 / M_PI); // Yaw (진행 방향)
    Serial.print("\t");
    Serial.print(ypr[1] * 180 / M_PI); // Pitch (앞뒤 기울기)
    Serial.print("\t");
    Serial.println(ypr[2] * 180 / M_PI); // Roll (좌우 기울기)
  }
}