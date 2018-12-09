/*

Holocron Metro 328 Firmware version 1.0
Brandon Allen
12/08/2018

Based on:
Holocron Particle Firmware V1.0
https://www.hackster.io/daveclarke/holocron-lamp-for-the-discerning-jedi-0d1f07
Dave Clarke
18/02/16

IR Proximty circuit modified from instructables
https://www.instructables.com/id/Simple-IR-proximity-sensor-with-Arduino
All other rights reserved.
*/
#include <EEPROM.h>
#include <Servo.h>

int readIR();           // prototype
void ServoControl();    // prototype
void ServoControlReset(); // prototype

Servo myservo;          // Create servo object

bool toggle = false;    // Used as a one shot
bool powerON = true;    // initial start up flag

int eepromAddr = 1;     // EEPROM address used to store the status of the lamp
int eepromValue = 0;    // initilze the EEPROM variable
int servoPin = 9;        // Servo on pin D9
int pos = 0;            // Initial servo position
int IRemitter = 10;     // IR Emiter LED on pin D10
int IRpin = A0;         // IR Photodiode on pin A0
int ambientIR;          // variable to store the IR coming from the ambient
int obstacleIR;         // variable to store the IR coming from the object
int value[10];          // variable to store the IR values
int distance = 0;       // variable that will tell if there is an obstacle or not
int mainLED = 11;       // Main LEDs on Pin D11
int closed = 20;        // Servo closed angle
int open = 155;         // Servo open angle

void setup()
{
  // Initialise pin modes and assign servo pin
  myservo.attach(servoPin);
  pinMode(mainLED, OUTPUT);
  pinMode(IRemitter, OUTPUT);

  eepromValue = EEPROM.read(eepromAddr);

  //initial states
  
  if (eepromValue == 0){
    myservo.write(closed);
    digitalWrite(mainLED, LOW);
    digitalWrite(IRemitter, LOW);
  }
  if (eepromValue == 1){
    toggle = !toggle;
    myservo.write(open);
    analogWrite(mainLED, 255);
    digitalWrite(IRemitter, LOW);
  }



}

void loop()
{
  distance = readIR(10);                                      // Read value from IR sensor and store in distance Variable
  if ((distance > 15) && toggle == false) // open up lamp and turn on the lights
  {
    for(pos = closed; pos <= open; pos++)                     // goes from 5 degrees to 115 degrees
    {                                                         // in steps of 1 degree
      ServoControl(pos);                                      // Set Servo Position and Control LED Brightness
    }
      ServoControlReset();// When finished, reset variables etc.
      EEPROM.update(eepromAddr, 1);
      delay(4);
  }
  if ((distance > 15) && toggle == true)  // turn off lamp and close
  {
    for(pos = open; pos > closed; pos--)                     // goes from 115 degrees to 5 degrees
    {
      ServoControl(pos);                                      // Set Servo Position and Control LED Brightness
    }
    ServoControlReset();                                      // When finished, reset variables etc.
    EEPROM.update(eepromAddr, 0);
    delay(4);
  }
}

// Function to read IR Proximity sensor
int readIR(int times)
{
  for(int x = 0; x < times; x++)
  {
    digitalWrite(IRemitter,LOW);    // turning the IR LEDs off to read the IR coming from the ambient
    delay(1);                       // minimum delay necessary to read values
    ambientIR = analogRead(IRpin);  // storing IR coming from the ambient
    digitalWrite(IRemitter,HIGH);   // turning the IR LEDs on to read the IR coming from the obstacle
    delay(1);                       // minimum delay necessary to read values
    obstacleIR = analogRead(IRpin); // storing IR coming from the obstacle
    value[x] = ambientIR-obstacleIR; // calculating changes in IR values and storing it for future average
  }
  for(int y = 0; y < times; y++)
  {
    distance+=value[y];              // calculating the average based on the "accuracy"
  }
  if (powerON == true)
  {
    delay(1000);                    // prevent bogus readings from servo noise on power up
    powerON = false;
  }
  else
  {
    //no start up delay
  }
  return(distance/times);            // return the final value
}

// Servo position control and LED brightness
void ServoControl(int ServoPosition)
{
  int LEDmap = ServoPosition;
  LEDmap = map(LEDmap, closed, open, 0, 255);  // Map servo position to LED brightness
  myservo.write(ServoPosition);                // tell servo to go to position in variable 'pos'
  delay(26);                                   // waits 25ms for the servo to reach the position
  analogWrite(mainLED, LEDmap);                //ramp light on using PWM Pin
}

// After servo has reached final position, reset variables and wait.
void ServoControlReset(void)
{
  toggle = !toggle;                           // toggle switch state
  distance = 0;                               // reest distance to stop accidental operation
  if (toggle == false)                        // Make sure LED is High for open and low for closed
  {
      digitalWrite(mainLED, LOW);
  }
  else
  {
     digitalWrite(mainLED, HIGH);
  }
  delay(1000);                                // prevent operation too quickly
}
