const int sensorPin = A0;       // TCRT5000 센서 아날로그 출력
const int ledPin = 13;          // 내장 LED (또는 외부 연결한 LED)

const int threshold = 500;      // 감지 기준값 (실험에 따라 조정 필요)

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(sensorPin);  // 센서 값 읽기

  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  if (sensorValue < threshold) {
    // 어두운 색 또는 물체 감지
    digitalWrite(ledPin, HIGH);  // LED ON
    Serial.println("Object Detected");
  } else {
    // 밝은 배경 (반사 강함)
    digitalWrite(ledPin, LOW);   // LED OFF
    Serial.println("No Object");
  }

  delay(1000);  // 0.2초 대기
}
