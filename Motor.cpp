#include "Motor.h"

Motor::Motor() {}

Motor::~Motor() {}

void Motor::motorInit() {
  pinMode(rotatestepPin, OUTPUT);
  pinMode(rotatedirPin, OUTPUT);
  pinMode(REN, OUTPUT);
  pinMode(RMS1, OUTPUT);
  pinMode(RMS2, OUTPUT);
  pinMode(RMS3, OUTPUT);

  digitalWrite(REN, HIGH); //turn rotate off
  set_sixteenthstep();

  Serial.println("motor initialized");
}

void Motor::set_fullstep() {
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

void Motor::set_quarterstep() {
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

void Motor::set_sixteenthstep() {
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

void Motor::rotateNSteps(int steps) {
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
