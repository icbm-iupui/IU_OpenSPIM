/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->	http://www.adafruit.com/products/1438
*/


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <AFMotor.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include "Tlc5940.h"

//lighting via TLC5940
TLC_CHANNEL_TYPE channel;





// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *pumpIn = AFMS.getMotor(4);
Adafruit_DCMotor *pumpOut = AFMS.getMotor(3);
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7
#define MOVEMENTDELAY 100
#define LIGHTON 2000
#define LIGHTOFF 0
#define NDFW 0
#define PERF 1


int menu = NDFW;

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 1);
int homeloop = 0;
int currentPosition = 0;
boolean perfusionState = false;
boolean perfusionForward = true;
boolean empty = false;
volatile double sensorValue;
String positionLabels[] = {"MIR ", "ND5  ", "ND4  ", "ND3  ", "ND2 ", "OPEN"};
String perfusionLabels[] = {"OFF", "ON FR", "EMPTY ", "ON RV"};
 
 
 
 

void setup() {
  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Stepper test!");
  Serial.println("Double coil");
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("IU OpenSPIM NDFW");
  
   
  lcd.setCursor(0, 1);
  lcd.print("Starting light...");
  
  Tlc.init();
  lightOn();

  
  AFMS.begin();  
    coarseHomeWheel();
    coarseHomeWheel();
   
    setMenu();
    
    delay(1000); 
     
     
  //run loop
}

void loop() {
 while(homeloop <= 12){
 readButtons();
 delay(MOVEMENTDELAY);
 }
 goToPosition(0);
 coarseHomeWheel();
}
  

 void coarseHomeWheel(){
   int lastPosition = currentPosition;
  lcd.setBacklight(RED);
  lcd.setCursor(0, 1);
  lcd.print("Homing wheel...     ");
  sensorValue = analogRead(A0);
  myMotor->setSpeed(50);  // 10 rpm   
    while(sensorValue<=50){
      lcd.print("Homing wheel...     ");
      sensorValue = analogRead(A0);
      delay(100);
      sensorValue = analogRead(A0);
      myMotor->step(1, FORWARD, DOUBLE); 
      sensorValue = analogRead(A0);
      currentPosition = 0;
      lcd.setCursor(0,1);
  }
  lcd.setBacklight(WHITE);
  lcd.setCursor(0,1);
  lcd.print("At Filter: " + positionLabels[currentPosition] + "     ");
  goToPosition(lastPosition);
  homeloop = 0;
 }
 
 void readButtons(){
   uint8_t buttons = lcd.readButtons();
   int selection = 0;
   

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if(menu == NDFW){
      if (buttons & BUTTON_UP) {
      lightOn();
      setMenu();
    }
    if (buttons & BUTTON_DOWN) {
      lightOff();
      setMenu();
    }
    if (buttons & BUTTON_LEFT) {
      setMenu();
      lcd.setCursor(0,1);
      moveBackward(true);
      lcd.setBacklight(WHITE);
    } 
    if (buttons & BUTTON_RIGHT) {
      setMenu();
      lcd.setCursor(0,1);
      moveForward(true);
      lcd.setBacklight(WHITE);
    }
    }
    if(menu == PERF){
    boolean b = !perfusionState;
    if (buttons & BUTTON_LEFT){
      stopPumps();
      setMenu();
    }
    if (buttons & BUTTON_RIGHT) {
      startPumps();
      setMenu();
    } 
    if (buttons & BUTTON_DOWN) {
      emptyChamber();
      empty = true;
      setMenu();
    }
    if (buttons & BUTTON_UP) {
      startPumpsReverse();
      setMenu();
    }
    }
    }
    if (buttons & BUTTON_SELECT) {
      if(menu == NDFW){menu = PERF;}
      else{menu = NDFW;}
      setMenu();
    }
 }
 

//Setup LCD menu
 void setMenu(){
   if(menu == NDFW){
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.setBacklight(WHITE);
   lcd.print("IU OpenSPIM NDFW");
   lcd.setCursor(0, 1);
    lcd.print("At Filter: " + positionLabels[currentPosition] + "     ");
   }
   else{
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.setBacklight(WHITE);
   lcd.print("IU OpenSPIM PERF");
   lcd.setCursor(0, 1);
   if(perfusionState){
     if(perfusionForward){
   lcd.print("Perfusion: " + perfusionLabels[1] + "     ");
     }
     else{
       lcd.print("Perfusion: " + perfusionLabels[3] + "     ");
     }
   }else if(empty){
     lcd.print("Perfusion: " + perfusionLabels[2] + "     ");
    
   }else{
     lcd.print("Perfusion: " + perfusionLabels[0] + "     ");
   }
   }
 
 }
 
 //move forward
 void moveForward(boolean count){
 myMotor->setSpeed(100); 
 myMotor->step(33, FORWARD, DOUBLE); 
 
 if(currentPosition == 5){
   currentPosition = 0;
 }
 else {
   currentPosition++;
 }
 lcd.print("At Filter: " + positionLabels[currentPosition] + "     ");
 if(count == true){homeloop++;}
 }
 
 //move backward
 void moveBackward(boolean count){
  myMotor->setSpeed(100); 
 myMotor->step(33, BACKWARD, DOUBLE); 
  
  
 if(currentPosition == 0){
   currentPosition = 5;
 }
 else {
   currentPosition--;
 }
 lcd.print("At Filter: " + positionLabels[currentPosition] + "     ");
 if(count == true){homeloop++;}
 }
 
 //goto select position
 boolean goToPosition(int position){     
       while(currentPosition != position){
         lcd.setBacklight(RED);
         lcd.setCursor(0,1);
         moveForward(false);
       }  
 return true;      
 }
 
 //turn BL off
 
 void lightOff(){
   for(int i = 0; i <= 20; i++){   
     Tlc.set(i, LIGHTOFF); 
     delay(5); 
     Tlc.update();    
  }
 }
 
 //turn BL on

 void lightOn(){ 
   for(int i = 0; i <= 20; i++){   
     Tlc.set(i, LIGHTON); 
     delay(5); 
     Tlc.update();    
  }
 }
 
 //start perfusion pumps
 
 void startPumps(){
 
 pumpIn->setSpeed(100); 
 pumpOut->setSpeed(115);
 pumpIn->run(FORWARD);
 pumpOut->run(BACKWARD);
 perfusionState = true;
 perfusionForward = true;
 empty = false;
 }
 
  //stop perfusion pumps
 
 void stopPumps(){
 
pumpIn->setSpeed(0);
 pumpOut->setSpeed(0);
 pumpIn->run(RELEASE);
 pumpOut->run(RELEASE);
 perfusionState = false;
 empty = false;
 }
 
  void startPumpsReverse(){
 
 pumpIn->setSpeed(255); 
 pumpOut->setSpeed(200);
 pumpIn->run(BACKWARD);
 pumpOut->run(FORWARD);
 perfusionState = true;
 perfusionForward = false;
 empty = false;
 }
 
 void emptyChamber(){
  pumpIn->setSpeed(0); 
  pumpOut->setSpeed(0); 
  pumpOut->run(RELEASE); 
  pumpIn->run(BACKWARD);
  pumpIn->setSpeed(65000);  
  perfusionState = false;
  empty = true;
 }
 

 

