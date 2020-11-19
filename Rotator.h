#pragma once
#include "Motor.h"

class Rotator: public Motor {
public:
  boolean rotateCont = 0;
  boolean rotateRand = 0;
  int randdelay = 5;
  
  void rotate_continuous_stop();
  void rotate_continuous(int dir);
  void rotate_random();
  void backandforth();
};

void IRAM_ATTR rotateTimerEvent_continuous();
