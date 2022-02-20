#include <Arduino.h>
#include <SPI.h>

// #define USE_CONSOLE

const int LED = 9;
const int SLAVE_SELECT = 10;

const int POT_POWER = 6;
const int POT = A6;

const int SWITCH_LEFT = 3;
const int SWITCH_RIGHT = 2;

const int JOY_POWER = 5;
const int L_JOY_VER = 20;
const int L_JOY_HOZ = 21;
const int R_JOY_VER = 18;
const int R_JOY_HOZ = 19;
const int L_JOY_SWITCH = 0;
const int R_JOY_SWITCH = 1;

#ifdef USE_CONSOLE
  char buffer[256];
#endif

#include "RobotCommand.h"

void sendUpdate(RobotCommand const& update) {
  digitalWrite(SLAVE_SELECT, LOW);
  byte const* data = (byte const*)&update;
  for (unsigned i=0; i < sizeof(RobotCommand); i++) {
    SPI.transfer(*data++);
  }
  digitalWrite(SLAVE_SELECT, HIGH);
}

void setup() {

  pinMode(SLAVE_SELECT, OUTPUT);
  digitalWrite(SLAVE_SELECT, HIGH);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(POT_POWER, OUTPUT);
  digitalWrite(POT_POWER, HIGH);
  pinMode(JOY_POWER, OUTPUT);
  digitalWrite(JOY_POWER, HIGH);

  pinMode(POT, INPUT);
  pinMode(SWITCH_LEFT, INPUT_PULLUP);
  pinMode(SWITCH_RIGHT, INPUT_PULLUP);
  pinMode(R_JOY_VER, INPUT);
  pinMode(R_JOY_HOZ, INPUT);
  pinMode(L_JOY_VER, INPUT);
  pinMode(L_JOY_HOZ, INPUT);
  pinMode(L_JOY_SWITCH, INPUT_PULLUP);
  pinMode(R_JOY_SWITCH, INPUT_PULLUP);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);

#ifdef USE_CONSOLE
  Serial.begin(115200);
#endif
}

void loop() {

  // Robot platform controls
  int drive = analogRead(L_JOY_VER);
  int steer = analogRead(L_JOY_HOZ);
  drive = map(drive, 0, 1023, 100, -100);
  steer = map(steer, 0, 1023, -100, 100);

  // Robot arm controls
  int shoulder = analogRead(POT);
  int elbow = analogRead(R_JOY_HOZ);
  int gripper = analogRead(R_JOY_VER);
  shoulder = map(shoulder, 0, 1023, -100, 100);
  elbow = map(elbow, 0, 1023, -100, 100);
  gripper = map(gripper, 0, 1023, 100, -100);

  int RotateL = !digitalRead(SWITCH_LEFT);
  int RotateR = !digitalRead(SWITCH_RIGHT);
  int SwitchL = !digitalRead(L_JOY_SWITCH);
  int SwitchR = !digitalRead(R_JOY_SWITCH);

  int yaw = (RotateL ? -100 : (RotateR ? 100 : 0));
  int spare = (SwitchL ? -100 : (SwitchR ? 100 : 0));

#ifdef USE_CONSOLE
  sprintf(buffer, "S=%4d, D=%4d, S=%4d, G=%4d, E=%4d", shoulder, drive, steer, gripper, elbow);
  Serial.print(buffer);
  sprintf(buffer,", RL=%d, RR=%d, SL=%d, SR=%d",RotateL,RotateR,SwitchL,SwitchR);
  Serial.println(buffer);
#endif

  // Send the command update to the robot
  RobotCommand update;
  update.drive = drive;
  update.steer = steer;
  update.yaw = yaw;
  update.shoulder = shoulder;
  update.elbow = elbow;
  update.gripper = gripper;
  update.buzzer = spare;
  printRobotCommand(update);
  sendUpdate(update);

  // Update every 100 ms
  digitalWrite(LED, !digitalRead(LED));
  delay(100);
}
