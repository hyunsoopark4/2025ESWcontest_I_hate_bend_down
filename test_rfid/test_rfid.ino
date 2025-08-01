#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10      // SDA / SS 핀
#define RST_PIN 9      // RST 핀

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();         // SPI 통신 시작
  rfid.PCD_Init();     // RFID 모듈 초기화
  Serial.println("RC522 준비 완료. 카드 태그를 대세요.");
}

void loop() {
  // 카드가 새로 인식되지 않았으면 리턴
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("카드 UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  rfid.PICC_HaltA(); // 태그 비활성화
}
