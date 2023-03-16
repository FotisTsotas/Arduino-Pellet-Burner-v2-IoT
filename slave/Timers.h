#ifndef TIMERS_H
#define TIMERS_H

#include <Arduino.h>

class Timers {
  public:
    Timers(int motorPelletPin);
    void throwingStandBy(int throwingTime, int standByTime);

  private:
    int motorPelletPin;
    int state;
    int time;
    unsigned long previousMillis;
};

#endif  // TIMER_H
