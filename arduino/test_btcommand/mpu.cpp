#include "mpu.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

// 전역 변수 정의
float yaw_initial = 0.0;
float last_valid_yaw = 0.0;  // 마지막 유효한 yaw 값 저장

// MPU6050 객체 및 관련 변수
MPU6050 mpu;
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

// MPU를 사용 가능한 상태로 초기화
void mpu_init() {
    Wire.begin();
    
    // MPU6050 초기화
    mpu.initialize();
    devStatus = mpu.dmpInitialize();
    
    // DMP 초기화 성공 시, 자이로 오프셋 보정(캘리브레이션) 진행
    if (devStatus == 0) {
        Serial.println(F("MPU DMP initialization successful"));
        mpu.setDMPEnabled(true);
        
        // 인터럽트 설정
        attachInterrupt(digitalPinToInterrupt(2), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();
        
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
        Serial.println(F("MPU ready"));
    } else {
        // 에러 코드 출력
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
        dmpReady = false;
    }
}

// 현재 yaw 값을 읽어오는 내부 함수
float getCurrentYaw() {
    if (!dmpReady) return last_valid_yaw;
    
    // 인터럽트가 발생할 때까지 기다리거나, FIFO 버퍼가 꽉 찼으면 처리
    unsigned long startTime = millis();
    while (!mpuInterrupt && fifoCount < packetSize) {
        // 타임아웃 처리 (1초)
        if (millis() - startTime > 1000) {
            Serial.println(F("MPU read timeout"));
            return last_valid_yaw;
        }
    }
    
    // 인터럽트 플래그 리셋 및 FIFO 상태 확인
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();
    fifoCount = mpu.getFIFOCount();
    
    // FIFO 오버플로우 확인
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        mpu.resetFIFO();
        return last_valid_yaw;
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
        
        // 라디안을 각도로 변환
        float current_yaw = ypr[0] * 180.0 / M_PI;
        last_valid_yaw = current_yaw;  // 유효한 값을 저장
        return current_yaw;
    }
    
    return last_valid_yaw;
}

// 안정적인 yaw 값을 기다리고 yaw_initial에 저장
void mpu_calibrate_yaw() {
    if (!dmpReady) {
        Serial.println(F("MPU not ready for calibration"));
        return;
    }
    
    Serial.println(F("Calibrating yaw... Please keep device stable"));
    
    const float STABILITY_THRESHOLD = 0.05;  // 안정성 임계값 (도)
    float samples[CALI_SAMPLE_COUNT];
    int sampleIndex = 0;
    bool calibrated = false;
    
    while (!calibrated) {
        float currentYaw = getCurrentYaw();
        samples[sampleIndex] = currentYaw;
        sampleIndex = (sampleIndex + 1) % CALI_SAMPLE_COUNT;
        
        // 충분한 샘플이 모이면 안정성 검사
        if (sampleIndex == 0) {  // 배열이 모두 채워졌을 때
            float sum = 0.0;
            float min_val = samples[0];
            float max_val = samples[0];
            
            // 평균, 최소, 최대값 계산
            for (int i = 0; i < CALI_SAMPLE_COUNT; i++) {
                if (samples[i] == 0.0) continue;
                if (samples[i] < min_val) min_val = samples[i];
                if (samples[i] > max_val) max_val = samples[i];
            }
            
            float range = max_val - min_val;
            
            // 값이 안정적인지 확인
            if (range < STABILITY_THRESHOLD) {
                yaw_initial = (max_val + min_val) / 2;
                calibrated = true;
                Serial.print(F("Yaw calibration complete. Initial yaw: "));
                Serial.println(yaw_initial);
            } else {
                Serial.print(F("Yaw unstable (range: "));
                Serial.print(range);
                Serial.println(F("), continuing calibration..."));
            }
        }
        
        delay(50);  // 50ms 간격으로 샘플링
    }
}

// 현재 yaw와 yaw_initial의 차이 계산 (-180 ~ 180)
float mpu_get_yaw_difference() {
    if (!dmpReady) return 0.0;
    
    float currentYaw = getCurrentYaw();
    // 오차 보정
    while(abs(currentYaw - getCurrentYaw()) > 1.0) {
        currentYaw = getCurrentYaw();
    }

    float difference = currentYaw - yaw_initial;

    // -180 ~ 180 범위로 정규화
    while (difference > 180.0) {
        difference -= 360.0;
    }
    while (difference < -180.0) {
        difference += 360.0;
    }
    
    return difference;
}
