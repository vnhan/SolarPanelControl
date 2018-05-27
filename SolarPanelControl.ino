/*===============================================================================
  Project : Control Solar Panel using Radio base on Arduino


  by Nguyen Van Nhan

  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
  ================================================================================*/

//Include library
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};


//Declare PIN & Default value
char eastLDRPin = A6;  //Assign analogue pins
char westLDRPin = A7;
#define enA 9
#define in1 5
#define in2 6
#define endstopEastPin 3
#define endstopWestPin 4
#define MAX_SPEED 255
#define EAST 1
#define WEST -1


//Declare Variable
int eastLDR = 0;
int westLDR = 0;
int angle = 0;
int error = 0;
int calibration = 10;  //Calibration offset to set error to zero when both sensors receive an equal amount of light
int maxTravel = 10000; //total time that panel finish hit endstop
int travelTime = 0;
int endstopWest = 0;
int endstopEast = 0;
int btnLeft = 0;
int btnRight = 0;
//int btnMode=0;
String mode = "A";
String sMode = "";
int panelAngle = 45;
boolean isEndstopEast = false;
boolean isEndstopWest = false;
String isRotate = "";
int currentSpeed = 0;
String currFunc="Read";
//int pwmOutput = 255; //range 0-255 : power manager - to control motor speed
//int motorDir = 1; // 1: rotate West | -1: rotate East

boolean isMotorBegin= true;
boolean isMotorMaxSpeed= false;
boolean isEndStopHit = false;
boolean isHome = false;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  Serial.println("Radio Init...");
  radio.begin();
  Serial.println("Radio Begin");
  radio.openWritingPipe(addresses[0]); // 00002
  radio.openReadingPipe(1, addresses[1]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  Serial.println("Radio Complete");
  calculateMaxTravelTime();
  Serial.println("Finish Init...");
}



void loop() {
  //start Program
  Serial.println("Start Program");
  readRadioCommand();
  writeRadioSensorData();
  checkEndstop();

  if (mode == "M") {
    manualControlPanel();
  } else if (mode == "A") {
    autoControlPanel();
  } else if (mode == "H") {
    autoHome();
  }

}

void rotateMotor(int motorDir, int motorSpeed) {
  //delay(50);

  if (motorDir == WEST && isMotorBegin) {
    //rotate West
    Serial.println("Start Rotate Motor WEST ");
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    isMotorBegin = false;
    isRotate = "<";
  } else if (motorDir == EAST && isMotorBegin) {
    //rotate East
    Serial.println("Start Rotate Motor EAST ");
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    isRotate = ">";
    isMotorBegin = false;
  }
  if (!isMotorMaxSpeed) {
    analogWrite(enA, currentSpeed); // Send PWM signal to L298N Enable pin
    Serial.println(currentSpeed);
  }
  
  if (currentSpeed <= motorSpeed-2) {
    currentSpeed += 2;
    //Serial.println(currentSpeed);
    delay(5);
  } else isMotorMaxSpeed = true;

}

void stopMotor() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(enA, 0);
  currentSpeed = 0;
  isRotate = "-";
  isMotorBegin=true;
  isMotorMaxSpeed=false;
}

void manualControlPanel() {
  //readRadioCommand();
  endstopEast = digitalRead(endstopEastPin);
  endstopWest = digitalRead(endstopWestPin);

  if (endstopEast == 1) {
    isEndstopEast = true;
  }

  if (endstopWest == 1) {
    isEndstopWest = true;
  }

  if (btnLeft == 1 && mode == "M" && endstopEast == 0 && endstopWest == 0) {
    //rotateWest();
    rotateMotor(WEST, MAX_SPEED);
    currFunc = "Sent";
  } else stopMotor();
  if (btnRight == 1 && mode == "M" && endstopEast == 0 && endstopWest == 0) {
    //rotateEast();
    rotateMotor(EAST, MAX_SPEED);
    currFunc = "Sent";
  } else stopMotor();

}

void checkEndstop () {
  Serial.println("Read Endstop");
  endstopEast = digitalRead(endstopEastPin);
  endstopWest = digitalRead(endstopWestPin);

  if (endstopEast == 1) {
    isEndstopEast = true;
    isRotate = "B";
    isEndStopHit = true;
  } else {
    isEndstopEast = false;
    isEndStopHit = false;
  }

  if (endstopWest == 1) {
    isEndstopWest = true;
    isRotate = "B";
    isEndStopHit = true;
  } else {
    isEndstopWest = false;
    isEndStopHit = false;
  }
}

void readRadioCommand() {
  delay(5);
  radio.startListening();
  if ( radio.available()) {
    while (radio.available()) {
      radio.read(&btnLeft, sizeof(btnLeft));
      radio.read(&btnRight, sizeof(btnRight));
      radio.read(&mode, sizeof(mode));
    }
  }
  sMode = mode;
}

void writeRadioSensorData() {
  delay(5);
  radio.stopListening();
  eastLDR = calibration + analogRead(eastLDRPin);    //Read the value of each of the east and west sensors
  westLDR = analogRead(westLDRPin);
  endstopEast = digitalRead(endstopEastPin);
  endstopWest = digitalRead(endstopWestPin);
  radio.write(&eastLDR, sizeof(eastLDR));
  radio.write(&westLDR, sizeof(westLDR));
  radio.write(&endstopEast, sizeof(endstopEast));
  radio.write(&endstopWest, sizeof(endstopWest));
  radio.write(&panelAngle, sizeof(panelAngle));
  radio.write(&sMode, sizeof(sMode));
  radio.write(&isRotate, sizeof(isRotate));
  radio.write(&currFunc, sizeof(currFunc));
}

void autoControlPanel() {
  
  Serial.println("Start Auto Control Funtion");
  eastLDR = calibration + analogRead(eastLDRPin);    //Read the value of each of the east and west sensors
  westLDR = analogRead(westLDRPin);
  //endstopEast = digitalRead(endstopEastPin);
  //endstopWest = digitalRead(endstopWestPin);
  checkEndstop();
  
  if (eastLDR < 350 && westLDR < 350 && !isEndStopHit) //Check if both sensors detect very little light, night time
  {
    autoHome();
    Serial.println("Return begin Angle");
  }
  error = eastLDR - westLDR;          //Determine the difference between the two sensors.
  if (error > 15)     //If the error is positive and greater than 15 then move the tracker in the east direction
  {
    if (travelTime < maxTravel && endstopWest == 0 && endstopEast == 0 && !isEndstopEast ) //Check that the tracker is not at the end of its limit in the east direction
    {
      //rotateEast();
      rotateMotor(EAST, MAX_SPEED);
      currFunc = "Rotate";
      travelTime--;
    } else stopMotor();
  }
  else if (error < -15) //If the error is negative and less than -15 then move the tracker in the west direction
  {
    if (travelTime < maxTravel && endstopWest == 0 && endstopEast == 0 && !isEndstopWest) //Check that the tracker is not at the end of its limit in the west direction
    {
      //rotateWest();
      rotateMotor(WEST, MAX_SPEED);
      currFunc = "Rotate";
      travelTime++;
    } else stopMotor();
  }
  //delay(100);
}



//Calculate all the time that solar panel need to hit 2 endstop
void calculateMaxTravelTime() {
  Serial.println("Calculate Max Travel Time");
  if (!isHome) {
    autoHome();
  }
  
  //Rotate Max
  Serial.println("Start calculate travel Time");
  checkEndstop();
  while (!isEndstopWest) //Move the tracker all the way back to face east for sunrise
  {
    //rotateWest();
    rotateMotor(WEST, MAX_SPEED);
    travelTime++;
    Serial.println(travelTime);
    //delay(100);
    //endstopWest = digitalRead(endstopWestPin);
    checkEndstop();
  } 
  if (isEndstopWest) {
    maxTravel = travelTime;
    stopMotor();
    Serial.print("Finish Calculate TravelTime :");
    Serial.println(maxTravel);
  }
  
}

void calculatePanelAngle() {
  if (travelTime == (maxTravel / 2)) {
    panelAngle = 0;
  } else if (travelTime < (maxTravel / 2)) {
    panelAngle = (travelTime * 45) / (maxTravel / 2);

  } else {
    panelAngle = ((travelTime * 45) / (maxTravel)) * -1;
  }
}

void autoHome() {
  
  Serial.println("Auto Home Function");
  checkEndstop();
  //endstopEast = digitalRead(endstopEastPin);
  //endstopWest = digitalRead(endstopWestPin);

  while (endstopEast == 0) //Move the tracker all the way back to face east for sunrise
  {
    //Serial.println("Start Rotate Motor EAST");
    rotateMotor(EAST, MAX_SPEED);
    travelTime--;
    //delay(100);
    endstopEast = digitalRead(endstopEastPin);
    Serial.println("Read Endstop EAST...");
  }
  if (endstopEast == 1) {
    travelTime = 0;
    panelAngle = 45;
    Serial.println("Endstop Hit");
    stopMotor();
    Serial.println("Finish Home");
    isHome = true;
  }
}


