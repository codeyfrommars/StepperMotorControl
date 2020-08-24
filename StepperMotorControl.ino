#define BLYNK_PRINT Serial

#define STEP_HIGH        REG_WRITE(GPIO_OUT_W1TS_REG, BIT0);
#define STEP_LOW         REG_WRITE(GPIO_OUT_W1TC_REG, BIT0);

#include <WiFi.h>
#include <WiFiClient.h>
#include <Blynk.h>
#include <BlynkSimpleEsp32.h>
#include "Rotation.h"

char auth[] = "";

char ssid[32] = "username";
char pass[32] = "password";

void setup()
{
  // Serial Monitor
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  
  rotateAccel = 100000; //2000 * sqrt( 2 * angle / accel )
  d = rotateAccel;
  Rotate_Init();
  rotatetimer = timerBegin(0, 80, true);
  timerAttachInterrupt(rotatetimer, &rotateTimerEvent, true);
  rotateconttimer = timerBegin(1, 80, true);
  timerAttachInterrupt(rotateconttimer, &rotateTimerEvent_continuous, true);
}



void loop() {
  Blynk.run();
  if (rotateEnable == 1) {
    if (rotateCont) {
    } else if (rotateRand) {
      rotate_random();
    } else {
      backandforth();
    }
  }

}

BLYNK_WRITE(V0)//enable rotation
{
    int pinValue = param.asInt();
    if (pinValue == 0) {
      digitalWrite(REN, HIGH);
      rotateEnable = 0;
    } else {
      digitalWrite(REN, LOW);
      rotateEnable = 1;
    }
}

BLYNK_WRITE(V1) {//rotateAccel 
    int val = param.asInt(); // assigning incoming value from pin V1 to a variable
    rotateAccel = val * 1000 * pow(0.7071, (log(microstep) / log(2)));
}
BLYNK_WRITE(V2) {//rotateAngle 
    int val = param.asInt(); // assigning incoming value from pin V1 to a variable
    rotateAngle = R_STEPS_PER_REV / 360.0 * val;
}
BLYNK_WRITE(V7) {//rotateSpd
    int val = param.asInt(); // assigning incoming value from pin V1 to a variable
    rotateSpd = val * 1000 / microstep;
}
BLYNK_WRITE(V3) {
  setR_sixteenthstep();
}


int tempspd;
int tempangle;
BLYNK_WRITE(V4) {
  if (rotateEnable) {
    int pinValue = param.asInt();
    if (pinValue == 0) {
      rotateRand = 0;
      rotateSpd = tempspd;
      rotateAngle = tempangle;
    } else {
      tempspd = rotateSpd;
      tempangle = rotateAngle;
      while (!movementDone); //wait for backandforth to finish
      delay(500);
      rotateRand = 1;
    }
  }
}
BLYNK_WRITE(V8) {//randdelay
    int val = param.asInt(); // assigning incoming value from pin V1 to a variable
    randdelay = val * 1000;
}


BLYNK_WRITE(V5) {//vibration
  setR_fullstep();
}
BLYNK_WRITE(V6) {//R_Continuous
  if (rotateEnable) {
    int pinValue = param.asInt();
    if (pinValue == 0) {
      rotate_continuous_stop();
    } else {
      while (!movementDone); //wait for backandforth to finish
      delay(500);
      rotateCont = 1;
      rotate_continuous(1); // add direction function (-1 or 1)
    }
  }
  
}
