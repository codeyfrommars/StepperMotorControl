#ifndef _ROTATION_H
#define _ROTATION_H

#include "Arduino.h"

const int rotateAcceldef = 100000;
const int rotatedirPin = 2;
const int rotatestepPin = 0;
const int RMS3 = 27;
const int RMS2 = 14;
const int RMS1 = 12;
const int REN = 13; //LOW = on, HIGH = off

//globals shared with main
extern boolean rotateEnable;
extern boolean rotateCont;
extern boolean rotateRand;
extern int R_STEPS_PER_REV;
extern int rotateDelay;
extern int rotateAngle;
extern int microstep;
extern int randdelay;

//for ISR
extern hw_timer_t * rotatetimer;
extern portMUX_TYPE rotatetimerMux;
extern hw_timer_t * rotateconttimer;
extern portMUX_TYPE rotateconttimerMux;
extern unsigned int rotateAccel;
extern volatile double d;
extern volatile int dir;
extern volatile unsigned int rotateSpd;
extern volatile unsigned long n;
extern volatile unsigned long stepCount;
extern volatile unsigned long rampUpStepCount;
extern volatile unsigned long totalSteps;
extern volatile int stepPosition;

extern volatile bool movementDone;

void setR_fullstep();

void setR_quarterstep();

void setR_sixteenthstep();

void Rotate_Init();

void IRAM_ATTR rotateTimerEvent();

void IRAM_ATTR rotateTimerEvent_continuous();

void rotateNSteps(int steps);

void rotate_continuous_stop();

void rotate_continuous(int dir);

void rotate_random();

void backandforth();





#endif // _ROTATION_H
