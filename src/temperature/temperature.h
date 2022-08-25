#ifndef B2F_TEMPERATURE_H // NOLINT(llvm-header-guard)
#define B2F_TEMPERATURE_H

#include "../enums.h"
#include "GyverPID.h"

class Temperature {
private:
  bool init;

  float kp = 20;
  float ki = 0.2;
  float kd = 1;
  int16_t dt = 100;

  int t2;
  int t3;
  int t4;
  int t5;
  int t6;
  int t7;
  int t8;
  int t9;
  int t10;
  int steinhartInt;

  uint8_t attachPCINT(uint8_t);

public:
  int targetTemp;
  Temperature() {
    if (!init) {
      GyverPID regulator(kp, ki, kd,
                         dt); // coefficient P, coefficient Et,
                              // coefficient. D, sampling period dt (ms)
      init = true;
    }
  }
  GyverPID regulator;
  int calculateSteinhart();
  void changeTargetTemp(Direction);
  void tempLoop();
  void setup();
};

#endif