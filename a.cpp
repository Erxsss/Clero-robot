// ============================================================
//   SMART CLEANING ROBOT — FULL POWER
//   L298N #1 → Left motor (Battery 1)
//   L298N #2 → Right motor (OUT1/OUT2) + Pump (OUT3/OUT4)
//   Pump pins: IN3=pin4, IN4=pin2, ENB=pin3
// ============================================================

#include <NewPing.h>

// ─────────────────────────────────────────────
// PINS
// ─────────────────────────────────────────────
const int L_IN1 = 8,  L_IN2 = 9,  L_EN = 5;   // Left motor  → L298N #1
const int R_IN1 = 10, R_IN2 = 11, R_EN = 6;   // Right motor → L298N #2 OUT1/OUT2
const int P_IN1 = 4,  P_IN2 = 2,  P_EN = 3;   // Pump        → L298N #2 OUT3/OUT4

const int FRONT_TRIG = A0, FRONT_ECHO = A1;
const int LEFT_TRIG  = A2, LEFT_ECHO  = A3;
const int RIGHT_TRIG = A4, RIGHT_ECHO = A5;

// ─────────────────────────────────────────────
// SPEED
// ─────────────────────────────────────────────
const int FULL       = 255;
const int CURVE_SLOW = 180;
const int TURN_POWER = 255;

// ─────────────────────────────────────────────
// TIMING
// ─────────────────────────────────────────────
const unsigned long TURN_90_MS       = 4000;
const unsigned long TURN_180_MS      = 5500;
const unsigned long SENSOR_INTERVAL  = 60;
const unsigned long PUMP_ON_DURATION = 2000;
const unsigned long PUMP_INTERVAL    = 20000;

// ─────────────────────────────────────────────
// SENSOR CONSTANTS
// ─────────────────────────────────────────────
const int MAX_DIST       = 200;
const int FRONT_OBSTACLE = 30;
const int WALL_TOLERANCE = 5;

// ─────────────────────────────────────────────
// ZIGZAG  20→40→60→80→60→40→20
// ─────────────────────────────────────────────
const int ZIGZAG_STEPS = 7;
const int zigzagDist[ZIGZAG_STEPS] = {20, 40, 60, 80, 60, 40, 20};
int  zigzagIndex = 0;
bool goingUp     = true;
bool zigzagDone  = false;

// ─────────────────────────────────────────────
// SENSORS
// ─────────────────────────────────────────────
NewPing frontSensor(FRONT_TRIG, FRONT_ECHO, MAX_DIST);
NewPing leftSensor (LEFT_TRIG,  LEFT_ECHO,  MAX_DIST);
NewPing rightSensor(RIGHT_TRIG, RIGHT_ECHO, MAX_DIST);
int distFront = 0, distLeft = 0, distRight = 0;
unsigned long lastSensorRead = 0;

// ─────────────────────────────────────────────
// PUMP STATE
// ─────────────────────────────────────────────
bool          pumpRunning      = false;
unsigned long pumpStartTime    = 0;
unsigned long lastAutoPumpTime = 0;

// ─────────────────────────────────────────────
// TURN STATE
// ─────────────────────────────────────────────
bool          isTurning   = false;
unsigned long turnEndTime = 0;

// ─────────────────────────────────────────────
// MODE
// ─────────────────────────────────────────────
bool autoMode = false;

// ─────────────────────────────────────────────
// MOTOR FUNCTIONS
// ─────────────────────────────────────────────
void leftForward()  { analogWrite(L_EN, FULL); digitalWrite(L_IN1, HIGH); digitalWrite(L_IN2, LOW);  }
void leftBackward() { analogWrite(L_EN, FULL); digitalWrite(L_IN1, LOW);  digitalWrite(L_IN2, HIGH); }
void leftStop()     { analogWrite(L_EN, 0);    digitalWrite(L_IN1, LOW);  digitalWrite(L_IN2, LOW);  }
void leftSlow()     { analogWrite(L_EN, CURVE_SLOW); digitalWrite(L_IN1, HIGH); digitalWrite(L_IN2, LOW); }

void rightForward()  { analogWrite(R_EN, FULL); digitalWrite(R_IN1, HIGH); digitalWrite(R_IN2, LOW);  }
void rightBackward() { analogWrite(R_EN, FULL); digitalWrite(R_IN1, LOW);  digitalWrite(R_IN2, HIGH); }
void rightStop()     { analogWrite(R_EN, 0);    digitalWrite(R_IN1, LOW);  digitalWrite(R_IN2, LOW);  }
void rightSlow()     { analogWrite(R_EN, CURVE_SLOW); digitalWrite(R_IN1, HIGH); digitalWrite(R_IN2, LOW); }

void moveForward()  { leftForward();  rightForward();  }
void moveBackward() { leftBackward(); rightBackward(); }
void stopMotors()   { leftStop();     rightStop();     }
void curveLeft()    { leftSlow();     rightForward();  }
void curveRight()   { leftForward();  rightSlow();     }

// ─────────────────────────────────────────────
// TURNS — non-blocking
// ─────────────────────────────────────────────
void startTurnLeft(unsigned long duration) {
  analogWrite(R_EN, TURN_POWER); analogWrite(L_EN, 0);
  digitalWrite(R_IN1, HIGH); digitalWrite(R_IN2, LOW);
  digitalWrite(L_IN1, LOW);  digitalWrite(L_IN2, LOW);
  isTurning   = true;
  turnEndTime = millis() + duration;
}

void startTurnRight(unsigned long duration) {
  analogWrite(L_EN, TURN_POWER); analogWrite(R_EN, 0);
  digitalWrite(L_IN1, HIGH); digitalWrite(L_IN2, LOW);
  digitalWrite(R_IN1, LOW);  digitalWrite(R_IN2, LOW);
  isTurning   = true;
  turnEndTime = millis() + duration;
}

void updateTurn() {
  if (!isTurning) return;
  if (millis() >= turnEndTime) {
    stopMotors();
    isTurning = false;
    delay(100);
  }
}

// ─────────────────────────────────────────────
// PUMP
// ─────────────────────────────────────────────
void pumpOn() {
  digitalWrite(P_IN1, HIGH);   // IN3 on L298N #2
  digitalWrite(P_IN2, LOW);    // IN4 on L298N #2
}
void pumpOff() {
  digitalWrite(P_IN1, LOW);
  digitalWrite(P_IN2, LOW);
  pumpRunning = false;
}
void startPump() {
  pumpOn();
  pumpRunning   = true;
  pumpStartTime = millis();
}
void updatePump() {
  if (pumpRunning && millis() - pumpStartTime >= PUMP_ON_DURATION) {
    pumpOff();
  }
}
void updateAutoPump() {
  if (!autoMode || zigzagDone) return;
  if (millis() - lastAutoPumpTime >= PUMP_INTERVAL) {
    lastAutoPumpTime = millis();
    startPump();
  }
}

// ─────────────────────────────────────────────
// SENSOR READ
// ─────────────────────────────────────────────
void readSensors() {
  if (millis() - lastSensorRead < SENSOR_INTERVAL) return;
  lastSensorRead = millis();
  distFront = frontSensor.ping_cm(); if (distFront == 0) distFront = MAX_DIST;
  distLeft  = leftSensor.ping_cm();  if (distLeft  == 0) distLeft  = MAX_DIST;
  distRight = rightSensor.ping_cm(); if (distRight == 0) distRight = MAX_DIST;
}

// ─────────────────────────────────────────────
// ADVANCE ZIGZAG
// ─────────────────────────────────────────────
void advanceZigzag() {
  if (goingUp) {
    zigzagIndex++;
    if (zigzagIndex >= ZIGZAG_STEPS - 1) {
      zigzagIndex = ZIGZAG_STEPS - 1;
      goingUp = false;
    }
  } else {
    zigzagIndex--;
    if (zigzagIndex <= 0) {
      zigzagDone = true;
      stopMotors();
      pumpOff();
      autoMode = false;
    }
  }
}

// ─────────────────────────────────────────────
// AUTO MODE
// ─────────────────────────────────────────────
void runAuto() {
  if (zigzagDone) return;
  if (isTurning)  return;

  readSensors();

  int  target      = zigzagDist[zigzagIndex];
  bool followRight = (zigzagIndex % 2 == 0);

  if (distFront < FRONT_OBSTACLE) {
    stopMotors();
    delay(150);
    followRight ? startTurnLeft(TURN_180_MS) : startTurnRight(TURN_180_MS);
    advanceZigzag();
    return;
  }

  int dist = followRight ? distRight : distLeft;

  if (dist < target - WALL_TOLERANCE) {
    followRight ? curveLeft() : curveRight();
  } else if (dist > target + WALL_TOLERANCE) {
    followRight ? curveRight() : curveLeft();
  } else {
    moveForward();
  }
}

// ─────────────────────────────────────────────
// MANUAL MODE
// ─────────────────────────────────────────────
void runManual() {
  if (!Serial.available()) return;
  char cmd = Serial.read();
  if (cmd < 'A' || cmd > 'S') return;

  switch (cmd) {
    case 'F': isTurning = false; moveForward();              break;
    case 'B': isTurning = false; moveBackward();             break;
    case 'L': startTurnLeft(TURN_90_MS);                     break;
    case 'R': startTurnRight(TURN_90_MS);                    break;
    case 'S': isTurning = false; stopMotors();               break;
    case 'P': startPump();                                   break;
    case 'O': pumpOff();                                     break;
    case 'A':
      autoMode         = true;
      zigzagDone       = false;
      zigzagIndex      = 0;
      goingUp          = true;
      isTurning        = false;
      lastAutoPumpTime = millis();
      moveForward();
      break;
    case 'M':
      autoMode  = false;
      isTurning = false;
      stopMotors();
      pumpOff();
      break;
  }
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  int pins[] = {L_IN1, L_IN2, L_EN, R_IN1, R_IN2, R_EN, P_IN1, P_IN2, P_EN};
  for (int i = 0; i < 9; i++) pinMode(pins[i], OUTPUT);
  analogWrite(P_EN, FULL);   // pump enable full power always
  stopMotors();
  pumpOff();
}

// ─────────────────────────────────────────────
// LOOP
// ─────────────────────────────────────────────
void loop() {
  if (Serial.available()) {
    char peek = Serial.peek();
    if (peek == 'M' || peek == 'A') { runManual(); return; }
  }
  updateTurn();
  updatePump();
  updateAutoPump();
  autoMode ? runAuto() : runManual();
}