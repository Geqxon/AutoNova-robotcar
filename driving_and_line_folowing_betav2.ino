
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Bluetooth: Arduino RX = D0, TX = D1
SoftwareSerial HM10(0, 1);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Line sensors
#define front1 A2
#define front2 A3

// Left motor direction pins
int in1 = 9;
int in2 = 8;

// Right motor direction pins
int in3 = 11;
int in4 = 10;

// Motor speed pins
int ena = 5;
int enb = 6;

// Speed levels
const int speedLevels[5] = {0, 64, 128, 191, 255};
int leftLevel = 3;
int rightLevel = 3;
bool motorsRunning = false;

// Ultrasonic sensor
long duration;
int distance;
const int trigPin = 2;
const int echoPin = 7;
const int obstacleDistance = 20;
const int reverseTime = 500;
const int turnTime = 600;

// Mode selection
enum Mode {
  REMOTE,
  SLAVE,
  AUTO
};

Mode currentMode = REMOTE;

// Function declarations
void Stop();
void Forward();
void Backward();
void TurnLeft();
void TurnRight();
void ApplySpeed();
int getDistance();
void AvoidObstacle();
void Modeswitching(char c);
void RemoteDrive(char c);
void AutoDrive();
void slavemode();
void setMode(Mode newMode);
void showMode();

// Setup
void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(front1, INPUT);
  pinMode(front2, INPUT);

  Stop();

  Serial.begin(9600);
  HM10.begin(9600);

  lcd.init();
  lcd.backlight();
  showMode();

  Serial.println("AutoNova-car ready for input.");
}

// Main loop
void loop() {
  if (HM10.available()) {
    char c = HM10.read();

    Serial.print("RECEIVED: ");
    Serial.println(c);

    distance = getDistance();

    // Serial.print("Distance: ");
    // Serial.print(distance);
    // Serial.println(" cm");

    Modeswitching(c);

    if (currentMode == REMOTE) {
      RemoteDrive(c);
    }

    if (currentMode == SLAVE) {
      slavemode();
    }

    // Triangle: increase both speed levels
    else if (c == 'E' || c == 'e') {
      if (leftLevel < 4) leftLevel++;
      if (rightLevel < 4) rightLevel++;
      ApplySpeed();
    }

    // X: decrease both speed levels
    else if (c == 'G' || c == 'g') {
      if (leftLevel > 1) leftLevel--;
      if (rightLevel > 1) rightLevel--;
      ApplySpeed();
    }
  }

  int f1 = digitalRead(front1);
  int f2 = digitalRead(front2);

  // Serial.print(f1);
  // delay(10);

  // Serial.println(f2);
  // delay(10);
}

// Line follower
void slavemode() {
  int f1 = digitalRead(front1);
  int f2 = digitalRead(front2);

  if (f1 == HIGH) {
    TurnRight();
  }
  else if (f2 == HIGH) {
    TurnLeft();
  }
  else {
    Forward();
  }
  return;
}

// Speed control
void ApplySpeed() {
  analogWrite(ena, motorsRunning ? speedLevels[leftLevel] : 0);
  analogWrite(enb, motorsRunning ? speedLevels[rightLevel] : 0);
}

void Forward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  motorsRunning = true;
  ApplySpeed();
  return;
}

void Backward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  motorsRunning = true;
  ApplySpeed();
  return;
}

void TurnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  motorsRunning = true;
  ApplySpeed();
  return;
}

void TurnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  motorsRunning = true;
  ApplySpeed();
  return;
}

// Ultrasonic distance
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  int distance = duration * 0.0343 / 2;
  return distance;
}

// Obstacle avoidance
void AvoidObstacle() {
  Serial.println("Obstacle detected!");
  Stop();
  delay(100);
}

// Mode switching
void Modeswitching(char c) {
  if (c == 'H' || c == 'h') {
    if (currentMode == REMOTE) {
      setMode(SLAVE);
    }
    else if (currentMode == SLAVE) {
      setMode(AUTO);
    }
    else {
      setMode(REMOTE);
    }
  }
}

// Remote drive
void RemoteDrive(char c) {
  if (c == 'A' || c == 'a') {
    Forward();
  }
  else if (c == 'C' || c == 'c') {
    Backward();
  }
  else if (c == 'B' || c == 'b') {
    TurnLeft();
  }
  else if (c == 'D' || c == 'd') {
    TurnRight();
  }
  else {
    Stop();
  }
}

// Autonomous mode
void AutoDrive() {
// emty couse needs work

}

// Mode display
void setMode(Mode newMode) {
  Stop();
  currentMode = newMode;
  showMode();
}

void showMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode:");

  lcd.setCursor(0, 1);

  if (currentMode == REMOTE) {
    lcd.print("REMOTE");
  }
  else if (currentMode == SLAVE) {
    lcd.print("SLAVE");
  }
  else {
    lcd.print("AUTO");
  }
}

// Stop motors
void Stop() {
  motorsRunning = false;
  ApplySpeed();

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}
