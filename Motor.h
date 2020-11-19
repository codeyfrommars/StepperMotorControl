#pragma once
#include "Arduino.h"


class Motor {
public:
  unsigned int rotateAccel = 100000;
  int rotatedirPin = 2;
  int rotatestepPin = 0;
  int RMS3 = 27;
  int RMS2 = 14;
  int RMS1 = 12;
  int REN = 13; //LOW = on, HIGH = off

  boolean rotateEnable = 0;
  int R_STEPS_PER_REV = 200;
  int rotateDelay = 1000;
  int rotateAngle = R_STEPS_PER_REV / 360.0 * 60;
  int microstep = 1;
  
  hw_timer_t * rotatetimer = NULL;
  portMUX_TYPE rotatetimerMux = portMUX_INITIALIZER_UNLOCKED;
  hw_timer_t * rotateconttimer = NULL;
  portMUX_TYPE rotateconttimerMux = portMUX_INITIALIZER_UNLOCKED;
  volatile double d = 100000;
  volatile int dir = 0; //direction
  volatile unsigned int rotateSpd = 10000;
  volatile unsigned long n = 0;
  volatile unsigned long stepCount = 0;
  volatile unsigned long rampUpStepCount = 0;
  volatile unsigned long totalSteps = 0;
  volatile int stepPosition = 0;
  volatile bool movementDone = false;


  Motor();
  ~Motor();
  //set microstep size
  void set_fullstep();
  void set_quarterstep();
  void set_sixteenthstep();
  //initialize motor
  void motorInit();
  void startTimer();
  void rotateNSteps(int steps);

  
};

void IRAM_ATTR rotateTimerEvent();
