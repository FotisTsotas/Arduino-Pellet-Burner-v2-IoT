#include "Usual.h"
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SI2C.h>
#include <EEPROM.h>

extern LiquidCrystal_I2C lcd;

void Usual::relay(bool air, bool resist, bool motorpellet, bool onofftimers) {
  digitalWrite(motorAir, air);
  digitalWrite(beginResistor, resist);
  digitalWrite(motorPellet, motorpellet);
  digitalWrite(onOffTimmers, onofftimers);
}

void Usual::errorOn(bool sel, int buzz) {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("PROVLIMA");  //ERROR
  digitalWrite(buzz, HIGH);
  delay(500);
  digitalWrite(buzz, LOW);
  delay(500);
  if (sel) {
    EEPROM.write(0, 1);
  }
}

void Usual::resetEppromReads() {
  EEPROM.write(1, 12);
  EEPROM.write(2, 3);
  EEPROM.write(3, 15);
  EEPROM.write(4, 3);
  EEPROM.write(5, 15);
  EEPROM.write(6, 2);
  EEPROM.write(7, 2);  //pellet Init
  EEPROM.write(8, 20);
  EEPROM.write(9, 3);
}

bool Usual::autoManual() {
  if (digitalRead(13)) {
    return true;
  } else {
    return false;
  }
}
