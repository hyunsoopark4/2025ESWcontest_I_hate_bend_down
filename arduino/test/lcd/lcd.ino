#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  
  lcd.begin();
  //lcd.backlight();
  //lcd.noBacklight();
  //delay(500);
  lcd.backlight();
  //lcd.setCursor(0, 0);

}

void loop() {
  
  //lcd.setCursor(0, 1); 
  lcd.home();
  lcd.clear();
  lcd.print("CHAOS");
  delay(2000);

  lcd.home();
  lcd.clear();
  lcd.print("INSERGENCY");
  delay(2000);

  lcd.home();
  lcd.clear();
  lcd.print("KEYCARD");
  delay(2000);
  //lcd.autoscroll();
}
