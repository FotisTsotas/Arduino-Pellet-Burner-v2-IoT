#include "Usual.h"
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SI2C.h>
#include <EEPROM.h>

extern LiquidCrystal_I2C lcd;

Usual::Usual(int onOffTimmers, int motorAir, int beginResistor, int motorPellet) {
  this->onOffTimmers = onOffTimmers;
  this->motorAir = motorAir;
  this->beginResistor = beginResistor;
  this->motorPellet = motorPellet;
}

void Usual::relay(bool air, bool resist, bool motorpel, bool onofftimer) {
  digitalWrite(motorAir, air);
  digitalWrite(beginResistor, resist);
  digitalWrite(motorPellet, motorpel);
  digitalWrite(onOffTimmers, onofftimer);
}

void Usual::errorOn(bool sel, int buzz) {
  Usual usual(7, 1, 2, 12); // Must be change 6 -> 12
  usual.relay(1, 1, 1, 1); 
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
  EEPROM.write(1, 12);// HIGH STAND
  EEPROM.write(2, 3); // HIGH THROW
  EEPROM.write(3, 15);
  EEPROM.write(4, 3);
  EEPROM.write(5, 15);
  EEPROM.write(6, 2);
  EEPROM.write(7, 2);  //pellet Init
  EEPROM.write(8, 20); //warming standby
  EEPROM.write(9, 4);//warming throw
}

bool Usual::autoManual() {
  if (digitalRead(13)) {
    return true;
  } else {
    return false;
  }
}
