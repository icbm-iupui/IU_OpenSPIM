/* Open-Spim Temperature and CO2 Perfusion Control
 *  
 *  Seth Winfree and Nathaniel Smith
 *  
 *  This code uses the following libraries and reference codes:
 *    The "Hello World" code from Adafruit's RGB LCD Shield Library
 *    "DC MotorTest" code from Adafruit's Motor Shield V2 Library
 *    "singleended" code from Adafruit's ADS1015 Library
 *    "PID_Basic" and "PID_AdaptiveTunings" code from the Arduino PID Library - Version 1.1.1 by Brett Beauregard
 *    "Using a Thermistor" code provided by Adafruit at https://learn.adafruit.com/thermistor/using-a-thermistor
 *    PID parameters were tuned according to the Classic Ziegler-Nichols Rule, which can be found at http://www.mstarlabs.com/control/znrule.html
 * 
 */

// Included Libraries
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <Adafruit_MotorShield.h>
#include <Adafruit_ADS1015.h>
#include <math.h>
#include <PID_v1.h>

// Initial Settings and Definitions
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
Adafruit_ADS1015 ads1 = Adafruit_ADS1015(0x48);    
Adafruit_ADS1015 ads2 = Adafruit_ADS1015(0x49); 
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60); 
Adafruit_DCMotor *pump = AFMS.getMotor(1);
int time = 0;
double steinhart0, steinhart1;
double adc0, adc1;

// This definition will be used to define the LCD backlight color
#define GREEN 0x2
#define WHITE 0x7

//Define the PID variables we'll be using
double Setpoint, Input, Output;
double minOutput = 0;
double maxOutput = 255;

//Define the PID tuning parameters
double aggKp=120, aggKi=60, aggKd=15;

//Specify the links and tuning parameters
PID myPID(&Input, &Output, &Setpoint, aggKp, aggKi, aggKd, DIRECT);

void setup() {

  Serial.begin(9600);
  ads1.begin();
  //ads2.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  Wire.begin();
  AFMS.begin();
    
  pump->run(FORWARD);
  //Serial.println("CLEARDATA");
  //Serial.println("LABEL, Timepoint, Time, T-Sample, T-Sample 2, Average Temperature, Error, Motor Setting (75-255)");

  //initialize the PID variables
  Input = 25;
  Setpoint = 37.1;

  // Use LCD to allow user to select a setpoint temperature
  uint8_t buttons = lcd.readButtons();
  lcd.setCursor(0,0);
  lcd.print(String("IU OpenSPIM ENVC"));
  lcd.setCursor(0,1);
  lcd.print(String("Set T(C):"));

  while (buttons != BUTTON_SELECT) {
    buttons = lcd.readButtons();
    lcd.setCursor(10,1);
    lcd.print(Setpoint);
    if (buttons & BUTTON_UP) {
      lcd.setCursor(10,1);
      Setpoint += .1;
      lcd.print(Setpoint);
    }
    if (buttons & BUTTON_DOWN) {
      lcd.setCursor(10,1);
      Setpoint -= .1;
      lcd.print(Setpoint);
    }
    delay(100);
  }
  
  //Setpoint += .05;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(minOutput,maxOutput); 
}

void loop() {

  // Read temperature sensors and calculate resistance
  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  double resistance0=2000/(1/((.003)*adc0)-.2);
  double resistance1=2000/(1/((.003)*adc1)-.2);
  
  delay(100);

  // Use resistance to calculate a temperature using the Steinhart-Hart B parameter equation method
  steinhart0 = resistance0/10000;     // (R/Ro)
  steinhart0 = log(steinhart0);                  // ln(R/Ro)
  steinhart0 /= 3950;                   // 1/B * ln(R/Ro)
  steinhart0 += 1.0 /( 25 + 273.15); // + (1/To)
  steinhart0 = 1.0 / steinhart0;                 // Invert
  steinhart0 -= 273.15;                         // convert to C
  steinhart1 = resistance1/10000;     // (R/Ro)
  steinhart1 = log(steinhart1);                  // ln(R/Ro)
  steinhart1 /= 3950;                   // 1/B * ln(R/Ro)
  steinhart1 += 1.0 /( 25 + 273.15); // + (1/To)
  steinhart1 = 1.0 / steinhart1;                 // Invert
  steinhart1 -= 273.15;                         // convert to C

  delay(100);

  // Display the temperature of the sample and reservoir on the LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String("IU OpenSPIM ENVC"));
  lcd.setCursor(0,1);
  lcd.print(String("Sample: ")+steinhart0);
  lcd.setCursor(13 ,1);  
  lcd.print(String("(C)"));
  //lcd.print(String("Air: ")+steinhart1);

  delay(1000);
  
    lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String("IU OpenSPIM ENVC"));
  lcd.setCursor(0,1);
  lcd.print(String("Resrv.: ")+steinhart1);
  lcd.setCursor(13 ,1);  
  lcd.print(String("(C)"));

  // Print the temperature of the sample and reservoir to serial
  time += 5;
  //Serial.print(time);
  //Serial.print(String("     ")+ steinhart0+String("      "));
  //Serial.print(steinhart1+String("   "));

  delay(100);
  
  Input = steinhart0;
  double Inputswitchpoint = .95*Setpoint;
    
  if (Input < Inputswitchpoint) {
    Output = 255;
    }
  
  // Calculate a PID output for the motor, given the temperature of the sample
  /*else*/ if (isnan(steinhart0) == 0) {  
  myPID.SetTunings(aggKp, aggKi, aggKd);

  myPID.Compute();

  // The peristaltic pump doesn't function at low voltages, so this redefines the output to low setting (45) or off (0)
  if (Output < 1) {
    Output = 0;
  }
  else if (Output < 45) { 
    Output = 45;
  }
    
  pump->setSpeed(Output); 

  delay(100);

  //Serial.print(Output);
  //Serial.println();

  //Serial.print("DATA,"); Serial.print("TIME,"); Serial.print(time); Serial.print(","); Serial.print(steinhart0); Serial.print(","); Serial.print(steinhart1); Serial.print(","); Serial.print((steinhart1+steinhart0)/2); Serial.print(","); Serial.print((fabs(steinhart1-steinhart0))/2); Serial.print(","); Serial.println(Output);
  
  delay(2500);
  }
  else {
    //Serial.println();
    delay(4600);
  }
}
