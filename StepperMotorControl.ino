#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <Blynk.h>
#include <BlynkSimpleEsp32.h>
#include "Rotator.h"

//Blynk authentication
char auth[] = "auth key";

char ssid[32] = "username";
char pass[32] = "password";


Rotator* cMotor;
//motor objects
Rotator rot1;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  
  rot1.motorInit();
  cMotor = &rot1;
  rot1.rotatetimer = timerBegin(0, 80, true);
  timerAttachInterrupt(rot1.rotatetimer, &rotateTimerEvent, true);
}

void loop() {
  Blynk.run();
  if (cMotor->rotateEnable == 1) {
    if (cMotor->rotateCont) {
    } else if (cMotor->rotateRand) {
      cMotor->rotate_random();
    } else {
      cMotor->backandforth();
    }
  }
}








void IRAM_ATTR rotateTimerEvent()
{
  //portENTER_CRITICAL_ISR(&rotatetimerMux);
  if ( cMotor->stepCount < cMotor->totalSteps ) {
    REG_WRITE(GPIO_OUT_W1TS_REG, BIT0); //STEP_HIGH reg write
    REG_WRITE(GPIO_OUT_W1TC_REG, BIT0); //STEP_LOW reg write
    cMotor->stepCount++;
  }
  else {
    cMotor->d = cMotor->rotateAccel;
    cMotor->movementDone = true;
    //portEXIT_CRITICAL_ISR(&rotatetimerMux);
    timerAlarmDisable(cMotor->rotatetimer);
  }
  if ( cMotor->rampUpStepCount == 0 ) { // ramp up phase
    cMotor->n++;
    cMotor->d = abs(cMotor->d - (2 * cMotor->d) / (4 * cMotor->n + 1));
    if ( cMotor->d <= cMotor->rotateSpd ) { // reached max speed
      cMotor->d = cMotor->rotateSpd;
      cMotor->rampUpStepCount = cMotor->stepCount;
    }
    if ( cMotor->stepCount >= cMotor->totalSteps / 2 ) { // reached halfway point
      cMotor->rampUpStepCount = cMotor->stepCount;
    }
  }
  else if ( cMotor->stepCount >= cMotor->totalSteps - cMotor->rampUpStepCount ) { // ramp down phase
    cMotor->d = abs((cMotor->d * (4 * cMotor->n + 1)) / (4 * cMotor->n + 1 - 2));
    cMotor->n--;
  }
  timerAlarmWrite(cMotor->rotatetimer, cMotor->d, true);
}

void IRAM_ATTR rotateTimerEvent_continuous()
{
  //portENTER_CRITICAL_ISR(&rotateconttimerMux);
  REG_WRITE(GPIO_OUT_W1TS_REG, BIT0); //STEP_HIGH reg write
  REG_WRITE(GPIO_OUT_W1TC_REG, BIT0); //STEP_LOW reg write
  if (cMotor->rotateCont == 0) { //ramp down phase
    cMotor->d = abs((cMotor->d * (4 * cMotor->n + 1)) / (4 * cMotor->n + 1 - 2));
    cMotor->n--;
  } else if ( cMotor->d > cMotor->rotateSpd ) { // ramp up phase
    cMotor->n++;
    cMotor->d = abs(cMotor->d - (2 * cMotor->d) / (4 * cMotor->n + 1));
  } else if (cMotor->d < cMotor->rotateSpd) {
    cMotor->d = abs((cMotor->d * (4 * cMotor->n + 1)) / (4 * cMotor->n + 1 - 2));
    cMotor->n--;
  } else {
    cMotor->d = cMotor->rotateSpd;
  }
  timerAlarmWrite(cMotor->rotateconttimer, cMotor->d, true);
}




//----------------------------Below are Blynk Functions-----------------------------------


BLYNK_WRITE(V0)//enable rotation
{
    int pinValue = param.asInt();
    if (pinValue == 0) {
      digitalWrite(cMotor->REN, HIGH);
      cMotor->rotateEnable = 0;
    } else {
      digitalWrite(cMotor->REN, LOW);
      cMotor->rotateEnable = 1;
    }
}

BLYNK_WRITE(V1) {//rotateAccel 
    int val = param.asInt(); // assigning incoming value from pin V1 to a variable
    cMotor->rotateAccel = val * 1000 * pow(0.7071, (log(cMotor->microstep) / log(2)));
}
BLYNK_WRITE(V2) {//rotateAngle 
    int val = param.asInt(); // assigning incoming value from pin V2 to a variable
    cMotor->rotateAngle = cMotor->R_STEPS_PER_REV / 360.0 * val;
}
BLYNK_WRITE(V7) {//rotateSpd
    int val = param.asInt(); // assigning incoming value from pin V7 to a variable
    cMotor->rotateSpd = val * 1000 / cMotor->microstep;
}
BLYNK_WRITE(V3) {//set 16th step
  cMotor->set_sixteenthstep();
}


static int tempspd;
static int tempangle;
BLYNK_WRITE(V4) {
  if (cMotor->rotateEnable) {
    int pinValue = param.asInt();
    if (pinValue == 0) {
      cMotor->rotateRand = 0;
      cMotor->rotateSpd = tempspd;
      cMotor->rotateAngle = tempangle;
    } else {
      tempspd = cMotor->rotateSpd;
      tempangle = cMotor->rotateAngle;
      while (!cMotor->movementDone); //wait for backandforth to finish
      delay(500);
      cMotor->rotateRand = 1;
    }
  }
}

BLYNK_WRITE(V8) {//randdelay
    int val = param.asInt();
    cMotor->randdelay = val * 1000;
}

BLYNK_WRITE(V5) {//set full step
  cMotor->set_fullstep();
}

BLYNK_WRITE(V6) {//R_Continuous
  if (cMotor->rotateEnable) {
    int pinValue = param.asInt();
    if (pinValue == 0) {
      cMotor->rotate_continuous_stop();
    } else {
      while (!cMotor->movementDone); //wait for backandforth to finish
      delay(500);
      cMotor->rotateCont = 1;
      cMotor->rotate_continuous(1); // add direction function (-1 or 1)
    }
  }
  
}
