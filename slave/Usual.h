#ifndef USUAL_H
#define USUAL_H

#include <Arduino.h>

class Usual {
public:
  Usual(int onOffTimmers, int motorAir, int beginResistor, int motorPellet);
  void relay(bool air, bool resist, bool motorpellet, bool onofftimers);
  static void errorOn(bool sel, int buzz);
  static bool autoManual();
  static void resetEppromReads();
  int motorAir;
  int beginResistor;
  int motorPellet;
  int onOffTimmers;

private:
};

#endif  // USUAL_H
