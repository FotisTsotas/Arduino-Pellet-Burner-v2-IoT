#include "Timers.h"
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SI2C.h>
#include <EEPROM.h>

extern LiquidCrystal_I2C lcd;

Timers::Timers(int motorPelletPin) {
  this->motorPelletPin = motorPelletPin;
  this->state = LOW;
  this->previousMillis = 0;
  time = 0;
  pinMode(motorPelletPin, OUTPUT);
}

void Timers::throwingStandBy(int throwingTime, int standByTime) {
  unsigned long currentMillis = millis();
  int onTime = throwingTime * 1000;
  int offTime = standByTime * 1000;
  if ((state == HIGH) && (currentMillis - previousMillis >= offTime)) {
    state = LOW;
    digitalWrite(motorPelletPin, LOW);
    previousMillis = currentMillis;
  } else if ((state == LOW) && (currentMillis - previousMillis >= onTime)) {
    state = HIGH;
    digitalWrite(motorPelletPin, HIGH);
    previousMillis = currentMillis;
  }
}