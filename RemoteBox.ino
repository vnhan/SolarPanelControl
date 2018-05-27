/*
  Project : Remote Box to control Solar Panel using Radio


  by Nguyen Van Nhan

  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

//Include library
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"

//Define pin & value
#define led 12
#define buttonLeft 1
#define buttonRight 2
#define buttonMode 3
#define DEFAULT_MODE "M";

//Declare device
// set the LCD address to 0x27 for a 16 chars 2 line display
// A FEW use address 0x3F

LiquidCrystal_I2C lcd(0x3F, 16, 2); // Set the LCD I2C address

RF24 radio(7, 8); // CE, CSN

//Declare global variable
const byte addresses[][6] = {"00001", "00002"};

int eastLDR = 0;
int westLDR = 0;
int angle = 0;
String mode = DEFAULT_MODE;

int endstopEast = 0;
int endstopWest = 0;
int panelAngle = 0;
int isBreak = 0;

int btnLeft = 0;
int btnRight = 0;
int btnMode = 0;
String isRotate = "";
String currFunc="";

//Start init
void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  
  
  Serial.println("Start initial");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Start Initial");
  delay(1000);
  
  //Declare Pinmode
  
  pinMode(led, OUTPUT);
  pinMode(buttonLeft, INPUT);
  pinMode(buttonRight, INPUT);
  pinMode(buttonMode, INPUT);

  radio.begin();
  Serial.println("Start Radio");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Start Radio");
  delay(1000);
  radio.openWritingPipe(addresses[1]); // 00001
  radio.openReadingPipe(1, addresses[0]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Finish Initial");
  delay(1000);
}

//Main app
void loop() {

  readBtn();
  readRadioData();
  writeRadioData();
  displaySolarPanelInfo();

}

//Function

void displaySolarPanelInfo() {
  String sensorInfo = "";
  String eastLDRstr(map(eastLDR,0,1023,0,100));
  String westLDRstr(map(westLDR,0,1023,0,100));
  String angleStr(angle);
  String angleInfo = "";
  String modeInfo="";
  lcd.clear();

  //Print line 1 LCD
  lcd.backlight();
  lcd.setCursor(0, 0);
  sensorInfo = "E/W:" + eastLDRstr + "/" + westLDRstr;
  Serial.println(sensorInfo);
  lcd.print(sensorInfo);
  angleInfo = "A" + angleStr;
  lcd.setCursor(13, 0);
  lcd.println(angleInfo);
  lcd.print(angleInfo);

  //Print line 2 LCD
  lcd.setCursor(0,1);
  if (mode = "M") {
    modeInfo = "Mode:M ";
  } else if (mode="A") {
    modeInfo = "Mode:A ";
  } else modeInfo = "Error ";
  lcd.print(modeInfo);

  lcd.setCursor(1,7);
  lcd.print(currFunc);

  lcd.setCursor(1,15);
  lcd.print(isRotate);
  
}

void readBtn () {
  btnLeft = digitalRead(buttonLeft);
  btnRight = digitalRead(buttonRight);
  btnMode = digitalRead(buttonMode);

  if(btnMode == HIGH) {
    changeMode();
  }
  
}

//Read data from SolarPanel
void readRadioData() {
  delay(5);
  radio.startListening();
  if ( radio.available()) {
    while (radio.available()) {
      radio.read(&eastLDR, sizeof(eastLDR));
      radio.read(&westLDR, sizeof(westLDR));
      radio.read(&endstopEast, sizeof(endstopEast));
      radio.read(&endstopWest, sizeof(endstopWest));
      radio.read(&panelAngle, sizeof(panelAngle));
      radio.read(&mode, sizeof(mode));
      radio.read(&isRotate, sizeof(isRotate));
      radio.read(&currFunc, sizeof(currFunc));

      if(currFunc =="") {
        currFunc = "Reading radio";
      }
    }
  }
}

//Send data to SolarPanel
void writeRadioData() {
  delay(5);
  radio.stopListening();

  radio.write(&btnLeft, sizeof(btnLeft));
  radio.write(&btnRight, sizeof(btnRight));
  radio.write(&mode, sizeof(mode));

}

void changeMode () {
  if (mode == "M") {
    mode = "A";
  } else if (mode=="A") {
    mode = "M";
  } else mode = "M";
}

