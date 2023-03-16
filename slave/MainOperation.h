#ifndef MAIN_OPERATION_H
#define MAIN_OPERATION_H

#include <Arduino.h>

class MainOperation {
  public:
    MainOperation(int onOffTimmers, int motorAir, int beginResistor, int motorPellet);
    void preFlameOperation(int fireValue, bool sel);
    static bool lowFire(float exhaustValue);
    static bool pelletIn(int endTime);
    bool warmingUp(int exhaustValue);
    bool endOperation;
    int buzz;
    int onOffTimmers;
    int motorPellet;
    int motorAir;
    int beginResistor;
  private:
    int time;
    unsigned long previousTime;
};

#endif // MAIN_OPERATION_H
