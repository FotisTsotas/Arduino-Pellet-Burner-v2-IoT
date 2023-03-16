#include "MainOperation.h"
#include "Usual.h"
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SI2C.h>
#include <avr/wdt.h>
#include <EEPROM.h>

extern Usual usual;
extern LiquidCrystal_I2C lcd;

MainOperation::MainOperation(int onOffTimmers, int motorAir, int beginResistor, int motorPellet) {
  this->onOffTimmers = onOffTimmers;
  this->motorAir = motorAir;
  this->beginResistor = beginResistor;
  this->motorPellet = motorPellet;
  previousTime = millis();
  time = 0;
}


void MainOperation::preFlameOperation(int fireValue, bool sel) {
  endOperation = false;
  unsigned long currentTime = millis();
  if (millis() - previousTime >= 1000) {
    time++;
    if (time >= 14 && time <= 15) {
      usual.relay(0, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("AERAS ANOIXTOS 1");
    } else if (time >= 18 && time <= 20) {
      usual.relay(0, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("AERAS ANOIXTOS 2");
    } else if (time >= 25 && time <= 30) {
      usual.relay(0, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("AERAS ANOIXTOS");
    } else if (time > 40) {
      usual.relay(1, 1, 1, 1);
      Usual::errorOn(sel, buzz);
    } else {
      usual.relay(1, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ANAFLEKSI ");
      lcd.print(time);
      lcd.print(" sec");
      lcd.setCursor(0, 1);
      lcd.print("FOTIA ");
      lcd.print(fireValue);
    }
    previousTime = currentTime;
  }
}

bool MainOperation::warmingUp(int exhaustValue) {
  time = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ZESTAMA ");
  lcd.setCursor(0, 1);
  lcd.print("KAYSAERIA ");
  lcd.print(exhaustValue);  //ktc.readCelsius()
  lcd.print(" C");
  endOperation = true;
  return endOperation;
}


bool MainOperation::pelletIn(int endTime) {
  while (endTime >= 0) {
    usual.relay(1, 1, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("EISAGWGI PELLET");
    lcd.setCursor(0, 1);
    lcd.print(endTime);
    lcd.print(" sec");
    endTime--;
    EEPROM.write(0, 0);
    delay(1000);
    wdt_reset();
  }
  EEPROM.write(0, 1);
  return;
}

bool MainOperation::lowFire(float exhaustValue) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("XAMILI ENAUSI");
  lcd.setCursor(0, 1);
  lcd.print("KAYSAERIA ");
  lcd.print(exhaustValue);  //ktc.readCelsius()
  lcd.print(" C");
  usual.relay(0, 0, 1, 1);
  return true;
}
