#ifndef _ROTATION_H
#define _ROTATION_H

const int rotateAcceldef = 100000;
const int rotatedirPin = 2;
const int rotatestepPin = 0;
const int RMS3 = 27;
const int RMS2 = 14;
const int RMS1 = 12;
const int REN = 13; //LOW = on, HIGH = off

//rotation
boolean rotateEnable = 0;
boolean rotateCont = 0;
boolean rotateRand = 0;
int R_STEPS_PER_REV = 200;
int rotateDelay = 1000;
int rotateAngle = R_STEPS_PER_REV / 360.0 * 60;
int microstep = 1;

//for interrupt routine
hw_timer_t * rotatetimer = NULL;
portMUX_TYPE rotatetimerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * rotateconttimer = NULL;
portMUX_TYPE rotateconttimerMux = portMUX_INITIALIZER_UNLOCKED;
unsigned int rotateAccel;
volatile double d;
volatile int dir = 0;
volatile unsigned int rotateSpd = 10000;
volatile unsigned long n = 0;
volatile unsigned long stepCount = 0;
volatile unsigned long rampUpStepCount = 0;
volatile unsigned long totalSteps = 0;
volatile int stepPosition = 0;

volatile bool movementDone = false;

void setR_fullstep() {
  if (microstep == 1) {
    return;
  }
  R_STEPS_PER_REV = 200;
  rotateAccel /= pow(0.7071, sqrt(microstep));
  rotateSpd *= microstep;
  rotateAngle /= microstep;
  digitalWrite(RMS1, LOW);
  digitalWrite(RMS2, LOW);
  digitalWrite(RMS3, LOW);
  microstep = 1;
}

void setR_quarterstep() {
  if (microstep == 4) {
    return;
  }
  R_STEPS_PER_REV = 800;
  if (microstep < 4) {
    rotateAccel *= pow(0.7071, (2 / microstep));
    rotateSpd /= 4 / microstep;
    rotateAngle *= 4 / microstep;
  } else {
    rotateAccel /= pow(0.7071, (microstep / 8));
    rotateSpd *= microstep / 4;
    rotateAngle /= microstep / 4;
  }
  digitalWrite(RMS1, LOW);
  digitalWrite(RMS2, HIGH);
  digitalWrite(RMS3, LOW);
  microstep = 4;
}

void setR_sixteenthstep() {
  if (microstep == 16) {
    return;
  }
  R_STEPS_PER_REV = 3200;
  rotateAccel *= pow(0.7071, 4 / sqrt(microstep));
  rotateSpd /= 16 / microstep;
  rotateAngle *= 16 / microstep;
  digitalWrite(RMS1, HIGH);
  digitalWrite(RMS2, HIGH);
  digitalWrite(RMS3, HIGH);
  microstep = 16;
}

void Rotate_Init() {
  
  pinMode(rotatestepPin, OUTPUT);
  pinMode(rotatedirPin, OUTPUT);
  pinMode(REN, OUTPUT);
  pinMode(RMS1, OUTPUT);
  pinMode(RMS2, OUTPUT);
  pinMode(RMS3, OUTPUT);

  digitalWrite(REN, HIGH); //turn rotate off
  setR_sixteenthstep();

}

void IRAM_ATTR rotateTimerEvent()
{
  //portENTER_CRITICAL_ISR(&rotatetimerMux);
  if ( stepCount < totalSteps ) {
    STEP_HIGH
    STEP_LOW
    stepCount++;
  }
  else {
    d = rotateAccel;
    movementDone = true;
    //portEXIT_CRITICAL_ISR(&rotatetimerMux);
    timerAlarmDisable(rotatetimer);
  }
  if ( rampUpStepCount == 0 ) { // ramp up phase
    n++;
    d = abs(d - (2 * d) / (4 * n + 1));
    if ( d <= rotateSpd ) { // reached max speed
      d = rotateSpd;
      rampUpStepCount = stepCount;
    }
    if ( stepCount >= totalSteps / 2 ) { // reached halfway point
      rampUpStepCount = stepCount;
    }
  }
  else if ( stepCount >= totalSteps - rampUpStepCount ) { // ramp down phase
    d = abs((d * (4 * n + 1)) / (4 * n + 1 - 2));
    n--;
  }
  timerAlarmWrite(rotatetimer, d, true);
}

void IRAM_ATTR rotateTimerEvent_continuous()
{
  //portENTER_CRITICAL_ISR(&rotateconttimerMux);
  STEP_HIGH
  STEP_LOW
  if (rotateCont == 0) { //ramp down phase
    d = abs((d * (4 * n + 1)) / (4 * n + 1 - 2));
    n--;
  } else if ( d > rotateSpd ) { // ramp up phase
    n++;
    d = abs(d - (2 * d) / (4 * n + 1));
  } else if (d < rotateSpd) {
    d = abs((d * (4 * n + 1)) / (4 * n + 1 - 2));
    n--;
  } else {
    d = rotateSpd;
  }
  timerAlarmWrite(rotateconttimer, d, true);
}

void rotateNSteps(int steps) {

  digitalWrite(rotatedirPin, steps < 0 ? HIGH : LOW);
  totalSteps = abs(steps);
  d = rotateAccel;
  timerAlarmWrite(rotatetimer, d, true);
  stepCount = 0;
  n = 0;
  rampUpStepCount = 0;
  movementDone = false;
  timerAlarmEnable(rotatetimer);
}

void rotate_continuous_stop() {
  rotateCont = 0;
  while (n > 0);
  timerAlarmDisable(rotateconttimer);
}

void rotate_continuous(int dir) {
  digitalWrite(rotatedirPin, dir < 0 ? HIGH : LOW);
  d = rotateAccel;
  timerAlarmWrite(rotateconttimer, d, true);
  n = 0;
  timerAlarmEnable(rotateconttimer);
}

int randdelay = 5;
void rotate_random() {
  rotateSpd = ((rand() % 12)+4) * 1000 / microstep;
  rotateNSteps(R_STEPS_PER_REV / 360.0 * (rand() % 721));
  while (!movementDone);
  delay(rand() % randdelay);
  rotateSpd = ((rand() % 12)+4) * 1000 / microstep;
  rotateNSteps(-(R_STEPS_PER_REV / 360.0 * (rand() % 721)));
  while (!movementDone);
  delay(rand() % randdelay);
}

void backandforth() {
  rotateNSteps(rotateAngle);
  while (!movementDone);
  delay(100);
  rotateNSteps(-rotateAngle);
  while (!movementDone);
  delay(100);
}





#endif // _ROTATION_H
