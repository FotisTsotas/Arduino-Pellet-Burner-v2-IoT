#ifndef MAIN_OPERATION_H
#define MAIN_OPERATION_H

#include <Arduino.h>

class MainOperation {
  public:
    MainOperation();
    void preFlameOperation(int fireValue, bool sel);
    static bool lowFire(float exhaustValue);
    bool endOperation;
    bool pellet;
    int buzz;
  private:
    int time;
    unsigned long previousTime;
};

#endif // MAIN_OPERATION_H
