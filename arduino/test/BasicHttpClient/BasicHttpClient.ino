/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>
#include <stdarg.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>


#define STASSID "lota"
#define STAPSK "37792292"
#define LCDWIDTH 16

char lcd_buffer1[LCDWIDTH+1];
char lcd_buffer2[LCDWIDTH+1];

ESP8266WiFiMulti WiFiMulti;
LiquidCrystal_I2C lcd(0x27, LCDWIDTH, 2);

void lcd_clearwrite(const char *s1 = NULL, const char *s2 = NULL)
{
  lcd.clear();
  if(s1 != NULL)
  {
    lcd.setCursor(0,0);
    lcd.print(s1);
  }
  if(s2 != NULL)
  {
    lcd.setCursor(0,1);
    lcd.print(s2);
  }
}

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  //LCD INIT  
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.home();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  lcd.print("WIFI CONNECTING");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\nConnected! IP address : ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.home();

  lcd_clearwrite("WIFI CONNECTED!", WiFi.localIP().toString().c_str());

  delay(10000);
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    lcd_clearwrite("[HTTP] begin ...");
    if (http.begin(client, "http://www.kma.go.kr/repositary/xml/fct/mon/img/fct_mon1rss_108_20250703.xml")) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        
        snprintf(lcd_buffer1,sizeof(lcd_buffer1),"[HTTP] GET : %d",httpCode);
        lcd_clearwrite(lcd_buffer1);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          payload = payload.substring(0, LCDWIDTH);  // LCD 한 줄 분량만 잘라냄
          Serial.println(payload);

          lcd.setCursor(0, 1);
          lcd.print(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.println("[HTTP] Unable to connect");
      lcd_clearwrite("[HTTP] Unable to connect");

    }
  }

  delay(20000);
}
