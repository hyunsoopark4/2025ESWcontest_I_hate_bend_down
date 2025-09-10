
#include <SoftwareSerial.h>

// HC-05:  RX<-10 , TX->11  (필요에 따라 핀 변경)
SoftwareSerial BT(10, 11); // RX, TX

// (선택) 라즈베리파이/외부 보드로 "시작" 신호를 주고 싶으면 한 핀을 트리거로 사용
const int TRIGGER_PIN = 7;   // 연결 안 쓰면 지워도 됨

void setup() {
  Serial.begin(9600);   // PC 디버깅용
  BT.begin(9600);       // HC-05 기본 통신속도
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW);

  BT.println("ARDUINO READY");
  Serial.println("READY");
}

String readLineFrom(Stream& s) {
  static String buf;
  while (s.available()) {
    char c = s.read();
    if (c == '\n') {        // 개행 기준
      String line = buf;
      buf = "";
      // 윈도우/안드로이드에서 올 수 있는 \r 제거
      line.trim();
      return line;
    } else {
      buf += c;
    }
  }
  return String();          // 아직 한 줄 완성 안 됨
}

void startRobot() {
  // >>> 여기서 '시작' 동작 수행 <<<
  // 1) 모터/서보/릴레이 켜기
  // 2) 또는 라즈베리파이 등 외부 컴퓨터에 신호 주기
  digitalWrite(TRIGGER_PIN, HIGH);
  delay(100);               // 펄스 100ms
  digitalWrite(TRIGGER_PIN, LOW);

  BT.println("OK START");
  Serial.println("OK START");
}

void stopRobot() {
  // >>> 여기서 '정지' 동작 수행 <<<
  BT.println("OK STOP");
  Serial.println("OK STOP");
}

void loop() {
  // 블루투스에서 한 줄 읽기
  String cmd = readLineFrom(BT);
  if (cmd.length() > 0) {
    Serial.print("CMD: "); Serial.println(cmd);

    if (cmd.equalsIgnoreCase("START")) {
      startRobot();
    } else if (cmd.equalsIgnoreCase("STOP")) {
      stopRobot();
    } else if (cmd.equalsIgnoreCase("STATUS")) {
      BT.println("STATUS OK");
    } else {
      BT.println("ERR UNKNOWN_CMD");
    }
  }

  // (선택) 주기적 상태 송신
  // BT.println("HB"); delay(1000);
}